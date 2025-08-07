#include <kurlyk.hpp>
#include <iostream>

// Helper function to print the response details
void print_response(const kurlyk::HttpResponsePtr& response) {
    KURLYK_PRINT
        << "Response received:" << std::endl
        << "Ready: " << response->ready << std::endl
        << "Content: " << response->content << std::endl
        << "Error Code: " << response->error_code << std::endl
        << "Status Code: " << response->status_code << std::endl
        << "----------------------------------------" << std::endl;
}

int main() {
    kurlyk::init(true);

    // Initialize the HTTP client
    kurlyk::HttpClient client("https://httpbin.org");
    client.set_user_agent("KurlykClient/1.0");
    client.set_verbose(true);

    // Send the first GET request
    KURLYK_PRINT << "Sending the first GET request..." << std::endl;
    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
        [&client](const kurlyk::HttpResponsePtr response) {
            print_response(response);

            if (response->ready && response->status_code == 200) {
                KURLYK_PRINT << "First request succeeded. Sending the second request..." << std::endl;

                // Send a second GET request within the callback of the first request
                client.get("/user-agent", kurlyk::QueryParams(), kurlyk::Headers(),
                    [](const kurlyk::HttpResponsePtr response) {
                        KURLYK_PRINT << "Second request completed." << std::endl;
                        print_response(response);
                    });
            } else {
                KURLYK_PRINT << "First request failed. Not sending the second request." << std::endl;
            }
        });

    std::cin.get();
    client.cancel_requests();
    kurlyk::deinit();
    return 0;
}
