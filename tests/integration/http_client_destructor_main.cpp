#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include "local_http_server.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

} // namespace

int main() {
    std::atomic<int> slow_hits(0);

    kurlyk_tests::LocalHttpServer server([&slow_hits](const kurlyk_tests::LocalHttpRequest& request) {
        if (request.method == "GET" && request.path == "/slow") {
            ++slow_hits;
            std::this_thread::sleep_for(std::chrono::seconds(3));
            return kurlyk_tests::make_http_response(200, "OK", "too-late");
        }
        return kurlyk_tests::make_http_response(404, "Not Found", "not-found");
    });
    server.start();

    std::mutex mutex;
    std::vector<long> statuses;
    std::vector<std::error_code> errors;

    kurlyk::init(true);
    {
        kurlyk::HttpClient client(server.host());
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

    std::cout << "HTTP client destructor cancellation integration test passed" << std::endl;
    return 0;
}
