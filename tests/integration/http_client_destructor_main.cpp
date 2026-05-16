#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <server_http.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

std::string make_response(long status, const std::string& reason, const std::string& body) {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << ' ' << reason << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Content-Type: text/plain\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return out.str();
}

} // namespace

int main() {
    HttpServer server;
    server.config.address = "127.0.0.1";
    server.config.port = 0;
    server.config.thread_pool_size = 1;

    std::atomic<int> slow_hits(0);
    server.resource["^/slow$"]["GET"] = [&slow_hits](std::shared_ptr<HttpServer::Response> response,
                                                       std::shared_ptr<HttpServer::Request>) {
        ++slow_hits;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        *response << make_response(200, "OK", "too-late");
    };

    const unsigned short port = server.bind();
    std::thread server_thread([&server]() {
        server.accept_and_run();
    });
    const std::string host = "http://127.0.0.1:" + std::to_string(port);

    std::mutex mutex;
    std::vector<long> statuses;
    std::vector<std::error_code> errors;

    kurlyk::init(true);
    {
        kurlyk::HttpClient client(host);
        client.set_rate_limit(1, 60000, kurlyk::RateLimitType::RL_GENERAL, true);

        auto callback = [&mutex, &statuses, &errors](kurlyk::HttpResponsePtr response) {
            std::lock_guard<std::mutex> lock(mutex);
            statuses.push_back(response ? response->status_code : 0);
            errors.push_back(response ? response->error_code : std::error_code());
        };

        require(client.get("/slow", kurlyk::QueryParams(), kurlyk::Headers(), callback),
                "First slow request was rejected");
        require(client.get("/slow", kurlyk::QueryParams(), kurlyk::Headers(), callback),
                "Second slow request was rejected");

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (slow_hits.load() == 0 && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        require(slow_hits.load() == 1, "First request did not reach the local HTTP server");

        // The HttpClient destructor should cancel all requests sharing this client's group_id:
        // one active request and one still blocked by the sequential rate limit.
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        require(statuses.size() == 2, "HttpClient destructor should deliver callbacks for both owned requests");
        require(std::count(statuses.begin(), statuses.end(), 499) == 2,
                "HttpClient destructor cancellation should return status 499 for both requests");
    }

    kurlyk::deinit();

    server.stop();
    server_thread.join();

    std::cout << "HTTP client destructor cancellation integration test passed" << std::endl;
    return 0;
}
