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
                     << "\n  Line: " << line;
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

    // ---

    for (int n = 0; n < 10; ++n) {
        KURLYK_PRINT << "N #" << n << "\n";

        uint32_t limit_id = kurlyk::create_rate_limit_rps(2);

        int num_clients = 10;
        int num_req = 3;
        int cancel_after_ms = 3000;

        std::vector<std::unique_ptr<kurlyk::HttpClient>> clients;
        std::vector<std::future<kurlyk::HttpResponsePtr>> futures;

        for (int i = 0; i < num_clients; ++i) {
            auto client = std::make_unique<kurlyk::HttpClient>("https://httpbin.org");
            client->set_timeout(5);
            client->set_connect_timeout(5);
            client->set_retry_attempts(3, 1000);

            KURLYK_PRINT << "Client #" << i << "\n";

            for (int j = 0; j < num_req; ++j) {
                if (j % 3 == 0) {
                    client->set_head_only(true);
                    futures.emplace_back(client->get("/delay/2", kurlyk::QueryParams(), kurlyk::Headers()));
                    client->set_head_only(false);
                } else {
                    futures.emplace_back(client->get("/delay/2", kurlyk::QueryParams(), kurlyk::Headers(), limit_id));
                }
            }
            clients.emplace_back(std::move(client));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(cancel_after_ms));

        for (int i = 0; i < num_clients; ++i) {

            auto& client = clients[i];

            KURLYK_PRINT << "Client #" << i << " using HEAD request\n";
            client->set_head_only(true);
            futures.emplace_back(client->get("/delay/2", kurlyk::QueryParams(), kurlyk::Headers()));
            client->set_head_only(false);

            try {
                KURLYK_PRINT << "[Cancel] Starting cancel for client #" << i << std::endl;
                client->cancel_requests();
                KURLYK_PRINT << "[Cancel] Finished cancel for client #" << i << std::endl;
            } catch (const std::exception& e) {
                KURLYK_PRINT << "[Cancel] Exception for client #" << i << ": " << e.what() << std::endl;
            }

            try {
                KURLYK_PRINT << "[Cancel2] Starting cancel for client #" << i << std::endl;
                client->cancel_requests();
                KURLYK_PRINT << "[Cancel2] Finished cancel for client #" << i << std::endl;
            } catch (const std::exception& e) {
                KURLYK_PRINT << "[Cancel2] Exception for client #" << i << ": " << e.what() << std::endl;
            }
        }

        KURLYK_PRINT << "Result:\n";
        for (int i = 0; i < num_clients; ++i) {
            try {
                auto response = futures[i].get();
                KURLYK_PRINT << "[Result] Client #" << i
                             << " | Ready: " << response->ready
                             << " | Status: " << response->status_code
                             << " | Error: " << response->error_code.message()
                             << std::endl;
            } catch (const std::exception& e) {
                KURLYK_PRINT << "[Result] Client #" << i << " threw exception: " << e.what() << std::endl;
            }
        }

        kurlyk::remove_limit(limit_id);
    }

    KURLYK_PRINT << "Exit?\n";
    std::cin.get();

    kurlyk::deinit();
    return 0;
}
