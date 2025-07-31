#include <iostream>
#include <kurlyk.hpp>

void print_response(const kurlyk::HttpResponsePtr& response) {
    KURLYK_PRINT
            << "ready: " << response->ready << std::endl
            << "response: " << std::endl
            << response->content << std::endl
            << "error_code: " << response->error_code << std::endl
            << "status_code: " << response->status_code << std::endl
            << "----------------------------------------" << std::endl;
}

int main() {
    kurlyk::HttpClient client("https://httpbin.org");

    KURLYK_PRINT << "Sending GET request using HttpClient method..." << std::endl;
    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
            [](const kurlyk::HttpResponsePtr response) {
        print_response(response);
    });

    KURLYK_PRINT << "Sending GET request using standalone function..." << std::endl;
    kurlyk::http_get("https://httpbin.org/ip", kurlyk::QueryParams(), kurlyk::Headers(),
            [](const kurlyk::HttpResponsePtr response) {
        print_response(response);
    });

    KURLYK_PRINT << "Press Enter to exit..." << std::endl;
    std::cin.get();
    kurlyk::deinit();
    return 0;
}
