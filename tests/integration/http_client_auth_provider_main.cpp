#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <server_http.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace {

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

void background_process(std::atomic<bool>& stop) {
    while (!stop.load()) {
        kurlyk::process();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

struct ProcessorGuard {
    std::atomic<bool> stop{false};
    std::thread thread;

    ProcessorGuard() : thread([this]() { background_process(stop); }) {}
    ~ProcessorGuard() {
        stop.store(true);
        if (thread.joinable()) thread.join();
    }
};

} // namespace

int main() {
    kurlyk::init(false);

    HttpServer server;
    server.config.port = 0;
    server.config.thread_pool_size = 2;

    std::string last_auth_header;
    server.resource["^/echo-auth$"]["GET"] = [&last_auth_header](
            std::shared_ptr<HttpServer::Response> response,
            std::shared_ptr<HttpServer::Request> request) {
        auto it = request->header.find("Authorization");
        if (it != request->header.end()) {
            last_auth_header = it->second;
        } else {
            last_auth_header.clear();
        }
        *response << "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok";
    };

    std::promise<unsigned short> port_promise;
    std::thread server_thread([&server, &port_promise]() {
        server.start([&port_promise](unsigned short port) {
            try {
                port_promise.set_value(port);
            } catch (...) {}
        });
    });

    const unsigned short port = port_promise.get_future().get();
    const std::string base_url = "http://127.0.0.1:" + std::to_string(port);

    {
        ProcessorGuard pg;

        // --- Test 1: BearerTokenAuthProvider injected via HttpClient ---
        {
            last_auth_header.clear();
            auto client = std::make_unique<kurlyk::HttpClient>(base_url);
            client->set_auth_provider(std::make_shared<kurlyk::http::auth::BearerTokenAuthProvider>(
                "test-token-123"));

            std::atomic<bool> callback_done{false};
            bool ok = client->get("/echo-auth", kurlyk::QueryParams(), kurlyk::Headers(),
                [&](kurlyk::HttpResponsePtr response) {
                    callback_done = true;
                });
            require(ok, "request should be accepted");
            client->wait_requests();
            require(callback_done.load(), "callback must be delivered");
            require(last_auth_header == "Bearer test-token-123",
                    "Authorization header must be injected by BearerTokenAuthProvider");
        }

        // --- Test 2: per-request Authorization header overwritten by provider ---
        {
            last_auth_header.clear();
            auto client = std::make_unique<kurlyk::HttpClient>(base_url);
            client->set_auth_provider(std::make_shared<kurlyk::http::auth::BearerTokenAuthProvider>(
                "provider-token"));

            std::atomic<bool> callback_done{false};
            kurlyk::Headers headers;
            headers.emplace("Authorization", "manual-token");
            bool ok = client->get("/echo-auth", kurlyk::QueryParams(), headers,
                [&](kurlyk::HttpResponsePtr response) {
                    callback_done = true;
                });
            require(ok, "request should be accepted");
            client->wait_requests();
            require(callback_done.load(), "callback must be delivered");
            require(last_auth_header == "Bearer provider-token",
                    "provider must overwrite per-request Authorization header");
        }

        // --- Test 3: set_auth_provider(nullptr) disables injection ---
        {
            last_auth_header.clear();
            auto client = std::make_unique<kurlyk::HttpClient>(base_url);
            client->set_auth_provider(std::make_shared<kurlyk::http::auth::BearerTokenAuthProvider>(
                "disabled-token"));
            client->set_auth_provider(nullptr);

            std::atomic<bool> callback_done{false};
            bool ok = client->get("/echo-auth", kurlyk::QueryParams(), kurlyk::Headers(),
                [&](kurlyk::HttpResponsePtr response) {
                    callback_done = true;
                });
            require(ok, "request should be accepted");
            client->wait_requests();
            require(callback_done.load(), "callback must be delivered");
            require(last_auth_header.empty(),
                    "Authorization must be absent when provider is nullptr");
        }
    }

    server.stop();
    server_thread.join();

    kurlyk::deinit();
    std::cout << "HttpClient auth provider integration test passed" << std::endl;
    return 0;
}
