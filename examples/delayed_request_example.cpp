#include <iostream>
#include <kurlyk.hpp>
#include <future>
#include <chrono>

void print_response(const kurlyk::HttpResponsePtr& response) {
    KURLYK_PRINT
        << "ready:              " << response->ready << std::endl
        << "response:           " << std::endl
        << response->content << std::endl
        << "error_code:         " << response->error_code << std::endl
        << "status_code:        " << response->status_code << std::endl
        << "retry_attempt:      " << response->retry_attempt << std::endl
        << "connect_time:       " << response->connect_time << std::endl
        << "appconnect_time:    " << response->appconnect_time << std::endl
        << "pretransfer_time:   " << response->pretransfer_time << std::endl
        << "starttransfer_time: " << response->starttransfer_time << std::endl
        << "total_time:         " << response->total_time << std::endl
        << "----------------------------------------" << std::endl;
}

int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    client.set_user_agent("KurlykClient/1.0");
    client.set_timeout(15); // Set a higher timeout to handle delayed responses
    client.set_connect_timeout(5);
    client.set_retry_attempts(2, 1000);

    int delay = 5; // Adjust delay as needed (in seconds)
    std::string path = "/delay/" + std::to_string(delay);

    // Using std::future for delayed GET request
    KURLYK_PRINT << "Sending delayed GET request with " << delay << " seconds delay..." << std::endl;
    auto future_response = client.get(path, kurlyk::QueryParams(), kurlyk::Headers());

    // Process response when it completes
    auto response = future_response.get();
    print_response(response);

    KURLYK_PRINT << "Delayed request completed." << std::endl;
    kurlyk::deinit();
    return 0;
}
