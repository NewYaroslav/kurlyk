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

std::unique_ptr<kurlyk::HttpRequest> make_limited_request(const kurlyk::HttpRateLimitHandlePtr& limit) {
#if __cplusplus >= 201402L
    auto request = std::make_unique<kurlyk::HttpRequest>();
#else
    auto request = std::unique_ptr<kurlyk::HttpRequest>(new kurlyk::HttpRequest());
#endif
    request->request_id = kurlyk::generate_request_id();
    request->group_id = kurlyk::generate_group_id();
    request->method = "GET";
    request->set_url("http://127.0.0.1", "/limited");
    request->specific_rate_limit = limit;
    return request;
}

} // namespace

int main() {
    kurlyk::init(false);

    auto limit = kurlyk::create_rate_limit(1, 60000);
    const long limit_id = limit->id();
    std::atomic<int> first_callback_count(0);
    std::atomic<int> second_callback_count(0);
    long second_status = 0;

    require(kurlyk::submit_http_request(
        make_limited_request(limit),
        [&first_callback_count](kurlyk::HttpResponsePtr) {
            ++first_callback_count;
        }).accepted, "First limited request was unexpectedly rejected");

    kurlyk::process();
    require(kurlyk::remove_limit(limit), "Removing a registered limit by handle failed");

    require(kurlyk::submit_http_request(
        make_limited_request(limit),
        [&second_callback_count, &second_status](kurlyk::HttpResponsePtr response) {
            second_status = response ? response->status_code : 0;
            ++second_callback_count;
        }).accepted, "Second limited request was unexpectedly rejected");

    kurlyk::process();
    require(second_callback_count.load() == 0,
            "Second request bypassed a removed manager-owned limit even though it held a copied handle");

    kurlyk::deinit();
    require(second_callback_count.load() == 1,
            "Pending request that held a copied rate-limit handle was not cleaned up during shutdown");
    require(second_status == 499, "Pending request cleanup must return status 499");

    limit.reset();
    require(!kurlyk::remove_limit(limit_id), "Limit registry entry survived after the last handle was released");

    std::cout << "HTTP rate limit lifetime integration test passed" << std::endl;
    return 0;
}
