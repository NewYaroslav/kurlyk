#include <iostream>
#include <kurlyk.hpp>
#include <future>

void print_response(const kurlyk::HttpResponsePtr& response) {
    KURLYK_PRINT
        << "ready: " << response->ready << std::endl
        << "response: " << std::endl
        << response->content << std::endl
        << "error_code: " << response->error_code << std::endl
        << "status_code: " << response->status_code << std::endl
        << "retry_attempt: " << response->retry_attempt << std::endl
        << "----------------------------------------" << std::endl;
}

int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    // Set client settings
    client.set_user_agent("KurlykClient/1.0");
    client.set_timeout(15);                // Request timeout (seconds)
    client.set_connect_timeout(5);         // Connection timeout (seconds)
    client.set_retry_attempts(3, 1000);    // 3 retry attempts, 1-second delay between retries
    client.set_rate_limit(5, 1000);        // Rate limit: 5 requests per second
    // Configure the maximum number of redirects
    int redirect_count = 15;  // Allow up to 15 redirects
    client.set_max_redirects(redirect_count);

    // Redirect handling - GET request with redirections
    KURLYK_PRINT << "Sending GET request to /absolute-redirect/" << redirect_count << "..." << std::endl;
    client.get("/absolute-redirect/" + std::to_string(redirect_count), kurlyk::QueryParams(), kurlyk::Headers(),
       [](const kurlyk::HttpResponsePtr response) {
           print_response(response);
       });

    KURLYK_PRINT << "Press Enter to exit..." << std::endl;
    std::cin.get();
    client.cancel_requests();
    kurlyk::deinit();
    return 0;
}

