#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

std::unique_ptr<kurlyk::HttpRequest> make_limited_request(
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

    const uint64_t request_id_a = kurlyk::generate_request_id();
    const uint64_t request_id_b = kurlyk::generate_request_id();
    const uint64_t request_id_c = kurlyk::generate_request_id();
    const uint64_t request_id_d = kurlyk::generate_request_id();
    const uint64_t group_id = kurlyk::generate_group_id();
    const uint64_t other_group_id = kurlyk::generate_group_id();

    require(request_id_a != request_id_b, "Request IDs must be unique");
    require(group_id != other_group_id, "Group IDs must be unique");

    auto limit = kurlyk::create_rate_limit(1, 60000);
    std::vector<long> statuses;

    auto collect_status = [&statuses](kurlyk::HttpResponsePtr response) {
        statuses.push_back(response ? response->status_code : 0);
    };

    require(kurlyk::submit_http_request(
        make_limited_request(limit, request_id_a, group_id, "/request-a"),
        collect_status).accepted, "Request A was unexpectedly rejected");
    require(kurlyk::submit_http_request(
        make_limited_request(limit, request_id_b, group_id, "/request-b"),
        collect_status).accepted, "Request B was unexpectedly rejected");
    require(kurlyk::submit_http_request(
        make_limited_request(limit, request_id_c, other_group_id, "/request-c"),
        collect_status).accepted, "Request C was unexpectedly rejected");
    require(kurlyk::submit_http_request(
        make_limited_request(limit, request_id_d, 0, "/request-d"),
        collect_status).accepted, "Request D was unexpectedly rejected");

    bool cancel_zero_done = false;
    kurlyk::cancel_requests_by_group_id(0, [&cancel_zero_done]() {
        cancel_zero_done = true;
    });
    kurlyk::process();
    require(cancel_zero_done, "Group zero cancellation callback was not called");
    require(statuses.empty(), "Group zero cancellation must not cancel ungrouped requests");

    bool cancel_one_done = false;
    kurlyk::cancel_request_by_id(request_id_a, [&cancel_one_done]() {
        cancel_one_done = true;
    });
    kurlyk::process();
    require(cancel_one_done, "Single request cancellation callback was not called");
    require(statuses.size() == 1, "Cancelling one request must cancel exactly one request");
    require(statuses.back() == 499, "Single request cancellation must return status 499");

    bool cancel_group_done = false;
    kurlyk::cancel_requests_by_group_id(group_id, [&cancel_group_done]() {
        cancel_group_done = true;
    });
    kurlyk::process();
    require(cancel_group_done, "Group cancellation callback was not called");
    require(statuses.size() == 2, "Group cancellation must cancel the remaining request in the group only");
    require(statuses.back() == 499, "Group cancellation must return status 499");

    bool cancel_other_group_done = false;
    kurlyk::cancel_requests_by_group_id(other_group_id, [&cancel_other_group_done]() {
        cancel_other_group_done = true;
    });
    kurlyk::process();
    require(cancel_other_group_done, "Other group cancellation callback was not called");
    require(statuses.size() == 3, "Other group cancellation must cancel exactly one request");
    require(statuses.back() == 499, "Other group cancellation must return status 499");

    kurlyk::deinit();
    require(statuses.size() == 4, "Ungrouped pending request should be cleaned up during shutdown");
    require(statuses.back() == 499, "Shutdown cleanup must return status 499");

    limit.reset();
    std::cout << "HTTP request/group cancellation integration test passed" << std::endl;
    return 0;
}
