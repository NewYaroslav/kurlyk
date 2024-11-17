#include <kurlyk.hpp>

void print_response(const kurlyk::HttpResponsePtr& response) {
    if (response->ready) {
        KURLYK_PRINT << "Response received:" << std::endl
                     << "Status Code: " << response->status_code << std::endl
                     << "Content: " << response->content << std::endl
                     << "----------------------------------------" << std::endl;
    } else {
        KURLYK_PRINT << "Response received:" << std::endl
                     << "Request not ready or cancelled." << std::endl
                     << "----------------------------------------" << std::endl;
    }
}

int main() {
    kurlyk::init(true);

    // Sending a GET request using the first function (callback-based)
    uint64_t request_id1 = kurlyk::http_get(
        "https://httpbin.org/delay/5",  // Delayed response to simulate long-running request
        kurlyk::QueryParams(),
        kurlyk::Headers(),
        [](kurlyk::HttpResponsePtr response) {
            KURLYK_PRINT << "Callback-based GET request response:" << std::endl;
            print_response(response);
        }
    );

#   if __cplusplus >= 201703L
    // Sending a GET request using the second function (future-based)
    auto [request_id2, future_response] = kurlyk::http_get(
        "https://httpbin.org/delay/5",  // Delayed response
        kurlyk::QueryParams(),
        kurlyk::Headers()
    );
#   else
    auto func_result = kurlyk::http_get(
        "https://httpbin.org/delay/5",  // Delayed response
        kurlyk::QueryParams(),
        kurlyk::Headers()
    );
    uint64_t request_id2 = func_result.first;
    auto future_response = std::move(func_result.second);
#   endif

    KURLYK_PRINT << "Sent two requests. Request IDs: " << request_id1 << ", " << request_id2 << std::endl;

    // Cancel the first request after a delay
    std::this_thread::sleep_for(std::chrono::seconds(1));
    KURLYK_PRINT << "Cancelling the first request (ID: " << request_id1 << ")..." << std::endl;
    kurlyk::cancel_request_by_id(request_id1, []() {
        KURLYK_PRINT << "Request 1 cancelled successfully." << std::endl;
    });

    // Cancel the second request after a delay
    KURLYK_PRINT << "Cancelling the second request (ID: " << request_id2 << ")..." << std::endl;
    kurlyk::cancel_request_by_id(request_id2).wait();

    // Ensure all operations complete before exiting
    try {
        future_response.get();
    } catch (const std::exception& e) {
        KURLYK_PRINT << "Future-based request exception: " << e.what() << std::endl;
    }

    kurlyk::deinit();
    return 0;
}
