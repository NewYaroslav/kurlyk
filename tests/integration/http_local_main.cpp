#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include "local_http_server.hpp"

#include <iostream>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

std::string header_value(const kurlyk_tests::LocalHttpRequest& request, const std::string& key) {
    auto it = request.headers.find(key);
    return it == request.headers.end() ? std::string() : it->second;
}

} // namespace

int main() {
    kurlyk_tests::LocalHttpServer server([](const kurlyk_tests::LocalHttpRequest& request) {
        if (request.method == "GET" && request.path == "/ok") {
            return kurlyk_tests::make_http_response(200, "OK", "local-ok");
        }
        if (request.method == "POST" && request.path == "/echo") {
            return kurlyk_tests::make_http_response(200, "OK", request.body);
        }
        if (request.method == "GET" && request.path == "/query") {
            return kurlyk_tests::make_http_response(200, "OK", request.query);
        }
        if (request.method == "GET" && request.path == "/headers") {
            return kurlyk_tests::make_http_response(200, "OK", header_value(request, "X-Kurlyk-Test"));
        }
        if (request.method == "GET" && request.path == "/head") {
            return kurlyk_tests::make_http_response(200, "OK", "head-body");
        }
        if (request.method == "HEAD" && request.path == "/head") {
            return kurlyk_tests::make_http_head_response(200, "OK", std::string("head-body").size());
        }
        if (request.method == "GET" && request.path == "/missing") {
            return kurlyk_tests::make_http_response(404, "Not Found", "missing");
        }
        if (request.method == "GET" && request.path == "/redirect") {
            return std::string("HTTP/1.1 302 Found\r\n") +
                   "Location: /ok\r\n" +
                   "Content-Length: 0\r\n" +
                   "Connection: close\r\n\r\n";
        }
        return kurlyk_tests::make_http_response(404, "Not Found", "not-found");
    });
    server.start();

    kurlyk::init(true);
    {
        kurlyk::HttpClient client(server.host());

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

    std::cout << "Local HTTP integration test passed" << std::endl;
    return 0;
}
