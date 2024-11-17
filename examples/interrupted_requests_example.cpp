#include <iostream>
#include <kurlyk.hpp>
#include <vector>

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

    // Set up client settings
    client.set_user_agent("KurlykClient/1.0");
    client.set_timeout(10);
    client.set_connect_timeout(5);

    std::vector<std::string> paths = {"/delay/3", "/delay/5", "/delay/7"};

    // Send multiple GET requests
    KURLYK_PRINT << "Sending multiple GET requests and immediately calling deinit()..." << std::endl;
    for (const auto& path : paths) {
        client.get(path, kurlyk::QueryParams(), kurlyk::Headers(),
           [](const kurlyk::HttpResponsePtr response) {
               print_response(response);
           });
    }

    // Immediately deinitialize the library to interrupt requests
    client.cancel_requests();
    kurlyk::deinit();
    KURLYK_PRINT << "Kurlyk library deinitialized. Waiting to see callback responses..." << std::endl;
    return 0;
}
