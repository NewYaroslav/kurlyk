#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

std::unique_ptr<kurlyk::HttpRequest> make_http_request(const std::string& path) {
#if __cplusplus >= 201402L
    auto request = std::make_unique<kurlyk::HttpRequest>();
#else
    auto request = std::unique_ptr<kurlyk::HttpRequest>(new kurlyk::HttpRequest());
#endif
    request->request_id = kurlyk::generate_request_id();
    request->method = "GET";
    request->set_url("http://127.0.0.1", path);
    return request;
}

} // namespace

int main() {
    kurlyk::init(false);
    kurlyk::set_max_pending_requests(1);

    std::atomic<int> accepted_callback_count(0);
    std::atomic<bool> rejected_submit_callback_called(false);
    std::atomic<bool> rejected_bool_callback_called(false);

    kurlyk::SubmitResult first_submit = kurlyk::submit_http_request(
        make_http_request("/accepted"),
        [&accepted_callback_count](kurlyk::HttpResponsePtr) {
            ++accepted_callback_count;
        });
    require(first_submit.accepted, "First HTTP submit was unexpectedly rejected");
    require(!first_submit.error_code, "Accepted HTTP submit returned an unexpected error code");

    kurlyk::SubmitResult second_submit = kurlyk::submit_http_request(
        make_http_request("/reject-submit"),
        [&rejected_submit_callback_called](kurlyk::HttpResponsePtr) {
            rejected_submit_callback_called = true;
        });
    require(!second_submit.accepted, "Second HTTP submit was expected to be rejected");
    require(second_submit.error_code == kurlyk::utils::make_error_code(kurlyk::utils::ClientError::QueueLimitExceeded),
            "Second HTTP submit returned an unexpected rejection code");
    require(!rejected_submit_callback_called.load(),
            "Rejected HTTP submit callback was called synchronously");

    const bool bool_submit = kurlyk::http_request(
        make_http_request("/reject-bool"),
        [&rejected_bool_callback_called](kurlyk::HttpResponsePtr) {
            rejected_bool_callback_called = true;
        });
    require(!bool_submit, "HTTP bool wrapper should reject when the queue is full");
    require(!rejected_bool_callback_called.load(),
            "Rejected HTTP bool wrapper callback was called synchronously");

    std::future<kurlyk::HttpResponsePtr> reject_future = kurlyk::http_request(
        make_http_request("/reject-future"));
    require(reject_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready,
            "Rejected HTTP future should become ready immediately");

    kurlyk::HttpResponsePtr response = reject_future.get();
    require(static_cast<bool>(response), "Rejected HTTP future returned a null response");
    require(response->ready, "Rejected HTTP future returned a non-ready response");
    require(response->status_code == 0, "Rejected HTTP future returned an unexpected status code");
    require(response->error_code == kurlyk::utils::make_error_code(kurlyk::utils::ClientError::QueueLimitExceeded),
            "Rejected HTTP future returned an unexpected error code");
    require(!response->error_message.empty(),
            "Rejected HTTP future returned an empty error message");

    kurlyk::set_max_pending_requests(0);
    kurlyk::deinit();

    require(!rejected_submit_callback_called.load(),
            "Rejected HTTP submit callback should never be called");
    require(!rejected_bool_callback_called.load(),
            "Rejected HTTP bool wrapper callback should never be called");
    require(accepted_callback_count.load() == 1,
            "Accepted HTTP request callback should be called once during shutdown cleanup");

    std::cout << "HTTP backpressure integration test passed" << std::endl;
    return 0;
}
