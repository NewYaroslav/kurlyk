#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <iostream>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

std::unique_ptr<kurlyk::HttpRequest> make_sequential_request(
        const kurlyk::HttpRateLimitHandlePtr& limit,
        uint64_t request_id,
        uint64_t group_id,
        const std::string& path) {
#if __cplusplus >= 201402L
    auto request = std::make_unique<kurlyk::HttpRequest>();
#else
    auto request = std::unique_ptr<kurlyk::HttpRequest>(new kurlyk::HttpRequest());
#endif
    request->request_id = request_id;
    request->group_id = group_id;
    request->method = "GET";
    request->set_url("http://127.0.0.1", path);
    request->specific_rate_limit = limit;
    return request;
}

} // namespace

int main() {
    kurlyk::init(false);

    const uint64_t request_id_1 = kurlyk::generate_request_id();
    const uint64_t request_id_2 = kurlyk::generate_request_id();
    const uint64_t group_id_1 = kurlyk::generate_group_id();
    const uint64_t group_id_2 = kurlyk::generate_group_id();

    auto limit = kurlyk::create_rate_limit(1, 60000, true);
    std::atomic<int> first_callback_count(0);
    std::atomic<int> second_callback_count(0);
    long first_status = 0;
    long second_status = 0;

    // Submit first request — it should be allowed through immediately.
    require(kurlyk::submit_http_request(
        make_sequential_request(limit, request_id_1, group_id_1, "/first"),
        [&first_callback_count, &first_status](kurlyk::HttpResponsePtr response) {
            first_status = response ? response->status_code : 0;
            ++first_callback_count;
        }).accepted, "First sequential request was unexpectedly rejected");

    // Submit second request — it must remain blocked because the first is still in-flight.
    require(kurlyk::submit_http_request(
        make_sequential_request(limit, request_id_2, group_id_2, "/second"),
        [&second_callback_count, &second_status](kurlyk::HttpResponsePtr response) {
            second_status = response ? response->status_code : 0;
            ++second_callback_count;
        }).accepted, "Second sequential request was unexpectedly rejected");

    // Move the first request to active; the second stays blocked in the pending queue.
    kurlyk::process();
    require(second_callback_count.load() == 0,
            "Second request started while the sequential limit was held by the first");

    // Cancel the first request by group_id. This triggers complete() and releases the sequential lock.
    bool cancel_done = false;
    kurlyk::cancel_requests_by_group_id(group_id_1, [&cancel_done]() {
        cancel_done = true;
    });
    kurlyk::process();
    require(cancel_done, "Cancellation callback for the first request was not called");
    require(first_callback_count.load() == 1,
            "First request callback was not called after cancellation");
    require(first_status == 499, "First request must return status 499 after cancellation");

    // The sequential lock is now released; the second request can start.
    kurlyk::process();
    require(second_callback_count.load() == 0,
            "Second request callback must not fire yet (connection refused, needs deinit cleanup)");

    // Clean up — deinit delivers the final callback for the second request.
    kurlyk::deinit();
    require(second_callback_count.load() == 1,
            "Second request callback was not called during shutdown cleanup");
    require(second_status == 499, "Second request must return status 499 after shutdown cleanup");

    std::cout << "Sequential rate limit integration test passed" << std::endl;
    return 0;
}
