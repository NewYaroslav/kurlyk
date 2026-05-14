#define KURLYK_AUTO_INIT 0
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

    kurlyk::add_error_handler([](const std::exception& ex,
                                 const char* func,
                                 const char* file,
                                 int line,
                                 const char* message) {
        KURLYK_PRINT << "Network error caught:"
                     << "\n  Message: " << message
                     << "\n  Exception: " << ex.what()
                     << "\n  Function: " << func
                     << "\n  File: " << file
                     << "\n  Line: " << line << std::endl;
    });

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

    const uint64_t group_id = kurlyk::generate_group_id();
    const uint64_t grouped_request_id1 = kurlyk::generate_request_id();
    const uint64_t grouped_request_id2 = kurlyk::generate_request_id();

    auto make_grouped_request = [group_id](uint64_t request_id) {
#       if __cplusplus >= 201402L
        auto request = std::make_unique<kurlyk::HttpRequest>();
#       else
        auto request = std::unique_ptr<kurlyk::HttpRequest>(new kurlyk::HttpRequest());
#       endif
        request->request_id = request_id;
        request->group_id = group_id;
        request->method = "GET";
        request->set_url("https://httpbin.org", "/delay/5");
        return request;
    };

    auto submit_grouped = [](std::unique_ptr<kurlyk::HttpRequest> request, const char* label) {
        kurlyk::SubmitResult submit = kurlyk::submit_http_request(
            std::move(request),
            [label](kurlyk::HttpResponsePtr response) {
                KURLYK_PRINT << label << " response:" << std::endl;
                print_response(response);
            });

        if (!submit) {
            KURLYK_PRINT << label << " submit rejected: " << submit.error_code.message() << std::endl;
        }
    };

    submit_grouped(make_grouped_request(grouped_request_id1), "Grouped request 1");
    submit_grouped(make_grouped_request(grouped_request_id2), "Grouped request 2");

    KURLYK_PRINT << "Sent grouped requests. Request IDs: "
                 << grouped_request_id1 << ", " << grouped_request_id2
                 << " | Group ID: " << group_id << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    KURLYK_PRINT << "Cancelling both grouped requests by group_id..." << std::endl;
    kurlyk::cancel_requests_by_group_id(group_id).wait();

    auto limit = kurlyk::create_rate_limit_rps(2);
    KURLYK_PRINT << "Created rate-limit handle ID: " << limit->id() << std::endl;

    kurlyk::HttpClient limited_client("https://httpbin.org");
    limited_client.set_timeout(5);
    limited_client.set_rate_limit_handle(limit);

    auto limited_future = limited_client.get("/delay/2", kurlyk::QueryParams(), kurlyk::Headers());
    kurlyk::remove_limit(limit);
    KURLYK_PRINT << "Released manager-owned rate-limit handle; pending requests keep copied handles alive." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    KURLYK_PRINT << "Cancelling the HttpClient request group..." << std::endl;
    limited_client.cancel_requests();

    try {
        print_response(limited_future.get());
    } catch (const std::exception& e) {
        KURLYK_PRINT << "Limited request exception: " << e.what() << std::endl;
    }

    KURLYK_PRINT << "Press Enter to exit..." << std::endl;
    std::cin.get();

    kurlyk::deinit();
    return 0;
}
