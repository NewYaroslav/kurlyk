#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <server_http.hpp>

#include <chrono>
#include <future>
#include <iostream>
#include <sstream>
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

std::string make_response(long status, const std::string& reason, const std::string& body) {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << ' ' << reason << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Content-Type: text/plain\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return out.str();
}

std::string header_value(const std::shared_ptr<HttpServer::Request>& request, const std::string& key) {
    auto it = request->header.find(key);
    return it == request->header.end() ? std::string() : it->second;
}

} // namespace

int main() {
    HttpServer server;
    server.config.port = 0;
    server.config.thread_pool_size = 1;

    server.resource["^/ok$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                            std::shared_ptr<HttpServer::Request>) {
        *response << make_response(200, "OK", "local-ok");
    };

    server.resource["^/echo$"]["POST"] = [](std::shared_ptr<HttpServer::Response> response,
                                             std::shared_ptr<HttpServer::Request> request) {
        const std::string body = request->content.string();
        *response << make_response(200, "OK", body);
    };

    server.resource["^/query$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                             std::shared_ptr<HttpServer::Request> request) {
        *response << make_response(200, "OK", request->query_string);
    };

    server.resource["^/headers$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                               std::shared_ptr<HttpServer::Request> request) {
        *response << make_response(200, "OK", header_value(request, "X-Kurlyk-Test"));
    };

    server.resource["^/head$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                            std::shared_ptr<HttpServer::Request>) {
        *response << make_response(200, "OK", "head-body");
    };

    server.resource["^/missing$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                               std::shared_ptr<HttpServer::Request>) {
        *response << make_response(404, "Not Found", "missing");
    };

    server.resource["^/redirect$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                std::shared_ptr<HttpServer::Request>) {
        *response << "HTTP/1.1 302 Found\r\n"
                  << "Location: /ok\r\n"
                  << "Content-Length: 0\r\n"
                  << "Connection: close\r\n\r\n";
    };

    std::promise<unsigned short> port_promise;
    auto port_future = port_promise.get_future();
    std::thread server_thread([&server, &port_promise]() {
        server.start([&port_promise](unsigned short port) {
            try {
                port_promise.set_value(port);
            } catch (...) {
            }
        });
    });

    const unsigned short port = port_future.get();
    const std::string host = "http://127.0.0.1:" + std::to_string(port);

    kurlyk::init(true);
    {
        kurlyk::HttpClient client(host);

        auto ok = client.get("/ok", kurlyk::QueryParams(), kurlyk::Headers()).get();
        require(ok && ok->ready, "GET /ok returned no final response");
        require(ok->status_code == 200, "GET /ok returned unexpected status");
        require(!ok->error_code, "GET /ok returned unexpected error_code");
        require(ok->content == "local-ok", "GET /ok returned unexpected body");

        auto post = client.post("/echo", kurlyk::QueryParams(), kurlyk::Headers(), "post-body").get();
        require(post && post->ready, "POST /echo returned no final response");
        require(post->status_code == 200, "POST /echo returned unexpected status");
        require(post->content == "post-body", "POST /echo did not echo request body");

        auto query = client.get("/query", kurlyk::QueryParams{{"q", "hello world"}, {"page", "1"}}, kurlyk::Headers()).get();
        require(query && query->ready, "GET /query returned no final response");
        require(query->status_code == 200, "GET /query returned unexpected status");
        require(query->content.find("q=hello%20world") != std::string::npos,
                "GET /query did not receive percent-encoded query parameter");
        require(query->content.find("page=1") != std::string::npos,
                "GET /query did not receive page query parameter");

        auto headers = client.get("/headers", kurlyk::QueryParams(),
            kurlyk::Headers{{"X-Kurlyk-Test", "header-ok"}}).get();
        require(headers && headers->ready, "GET /headers returned no final response");
        require(headers->status_code == 200, "GET /headers returned unexpected status");
        require(headers->content == "header-ok", "GET /headers did not forward custom header");

        client.set_head_only(true);
        auto head = client.get("/head", kurlyk::QueryParams(), kurlyk::Headers()).get();
        require(head && head->ready, "head-only request returned no final response");
        require(head->status_code == 200, "head-only request returned unexpected status");
        require(head->content.empty(), "head-only request should not return a body");
        client.set_head_only(false);

        auto missing = client.get("/missing", kurlyk::QueryParams(), kurlyk::Headers()).get();
        require(missing && missing->ready, "GET /missing returned no final response");
        require(missing->status_code == 404, "GET /missing returned unexpected status");
        require(static_cast<bool>(missing->error_code), "GET /missing should set an HTTP error_code");

        auto redirect = client.get("/redirect", kurlyk::QueryParams(), kurlyk::Headers()).get();
        require(redirect && redirect->ready, "GET /redirect returned no final response");
        require(redirect->status_code == 200, "GET /redirect did not follow redirect to /ok");
        require(redirect->content == "local-ok", "GET /redirect returned unexpected body");
    }
    kurlyk::deinit();

    server.stop();
    server_thread.join();

    std::cout << "Local HTTP integration test passed" << std::endl;
    return 0;
}
