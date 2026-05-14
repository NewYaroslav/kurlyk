#include <iostream>
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

    // Create a sequential rate limit: only one request may be in-flight at a time.
    // Retries of the same request also hold the lock; other requests wait.
    auto limit = kurlyk::create_rate_limit(1, 60000, true);
    KURLYK_PRINT << "Created sequential rate-limit handle ID: " << limit->id() << std::endl;

    const uint64_t group_id_1 = kurlyk::generate_group_id();
    const uint64_t group_id_2 = kurlyk::generate_group_id();

    auto make_sequential_request = [&limit](uint64_t group_id, const std::string& path) {
#if __cplusplus >= 201402L
        auto request = std::make_unique<kurlyk::HttpRequest>();
#else
        auto request = std::unique_ptr<kurlyk::HttpRequest>(new kurlyk::HttpRequest());
#endif
        request->request_id = kurlyk::generate_request_id();
        request->group_id = group_id;
        request->method = "GET";
        request->set_url("http://127.0.0.1", path);
        request->specific_rate_limit = limit;
        return request;
    };

    // Request 1: starts immediately (connection refused, but holds the sequential lock).
    KURLYK_PRINT << "Submitting Request 1 (group " << group_id_1 << ")..." << std::endl;
    kurlyk::submit_http_request(
        make_sequential_request(group_id_1, "/first"),
        [](kurlyk::HttpResponsePtr response) {
            KURLYK_PRINT << "Request 1 done (status " << response->status_code << ")" << std::endl;
            print_response(response);
        });

    // Request 2: blocked while Request 1 is in-flight.
    KURLYK_PRINT << "Submitting Request 2 (group " << group_id_2 << ") — waiting..." << std::endl;
    kurlyk::submit_http_request(
        make_sequential_request(group_id_2, "/second"),
        [](kurlyk::HttpResponsePtr response) {
            KURLYK_PRINT << "Request 2 done (status " << response->status_code << ")" << std::endl;
            print_response(response);
        });

    // Cancel Request 1 to release the sequential lock so Request 2 can proceed.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    KURLYK_PRINT << "Cancelling Request 1 by group_id..." << std::endl;
    kurlyk::cancel_requests_by_group_id(group_id_1).wait();
    KURLYK_PRINT << "Request 1 cancelled; sequential lock released." << std::endl;

    // Give Request 2 a chance to run and then cancel it too.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    KURLYK_PRINT << "Cancelling Request 2 by group_id..." << std::endl;
    kurlyk::cancel_requests_by_group_id(group_id_2).wait();

    KURLYK_PRINT << "Press Enter to exit..." << std::endl;
    std::cin.get();
    kurlyk::deinit();
    return 0;
}
