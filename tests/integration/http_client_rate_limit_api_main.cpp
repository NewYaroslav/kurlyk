#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <chrono>
#include <iostream>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

void drain_for(std::chrono::milliseconds duration) {
    const auto deadline = std::chrono::steady_clock::now() + duration;
    while (std::chrono::steady_clock::now() < deadline) {
        kurlyk::process();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

} // namespace

int main() {
    kurlyk::init(false);

    auto limit = kurlyk::create_rate_limit(1, 60000);
    const long limit_id = limit->id();

    kurlyk::HttpClient handle_client("http://127.0.0.1");
    require(handle_client.set_rate_limit_handle(limit), "set_rate_limit_handle rejected a valid handle");
    require(handle_client.assign_rate_limit_handle(limit), "assign_rate_limit_handle rejected a valid handle");
    require(!handle_client.set_rate_limit_handle(kurlyk::HttpRateLimitHandlePtr()),
            "set_rate_limit_handle accepted an empty handle");

    std::atomic<int> first_handle_callbacks(0);
    std::atomic<int> second_handle_callbacks(0);
    std::atomic<int> id_lookup_callbacks(0);
    long second_handle_status = 0;

    require(handle_client.get("/handle-1", kurlyk::QueryParams(), kurlyk::Headers(),
        [&first_handle_callbacks](kurlyk::HttpResponsePtr) {
            ++first_handle_callbacks;
        }), "First handle-client request was rejected");

    drain_for(std::chrono::milliseconds(50));
    require(kurlyk::remove_limit(limit_id), "remove_limit(id) failed for a registered handle");

    require(handle_client.get("/handle-2", kurlyk::QueryParams(), kurlyk::Headers(), limit,
        [&second_handle_callbacks, &second_handle_status](kurlyk::HttpResponsePtr response) {
            second_handle_status = response ? response->status_code : 0;
            ++second_handle_callbacks;
        }), "Per-request handle overload was rejected");

    drain_for(std::chrono::milliseconds(50));
    require(second_handle_callbacks.load() == 0,
            "Per-request handle overload bypassed throttling after manager-owned handle removal");

    kurlyk::HttpClient id_client("http://127.0.0.1");
    require(!id_client.set_rate_limit_id(limit_id), "set_rate_limit_id found a removed manager-owned handle");
    require(!id_client.assign_rate_limit_id(limit_id), "assign_rate_limit_id found a removed manager-owned handle");

    require(id_client.get("/id-lookup", kurlyk::QueryParams(), kurlyk::Headers(), limit_id,
        [&id_lookup_callbacks](kurlyk::HttpResponsePtr) {
            ++id_lookup_callbacks;
        }), "Per-request ID overload was rejected");

    drain_for(std::chrono::milliseconds(50));

    kurlyk::deinit();
    require(id_lookup_callbacks.load() == 1,
            "Per-request ID overload callback was not delivered during shutdown");
    require(second_handle_callbacks.load() == 1, "Pending handle-limited request was not cleaned up during shutdown");
    require(second_handle_status == 499, "Pending handle-limited cleanup must return status 499");

    limit.reset();
    std::cout << "HTTP client rate limit API integration test passed" << std::endl;
    return 0;
}
