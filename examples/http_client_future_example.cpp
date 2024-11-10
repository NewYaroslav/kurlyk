#include <iostream>
#include <kurlyk.hpp>
#include <future>

int main() {
    kurlyk::HttpClient client("https://httpbin.org");

    KURLYK_PRINT << "Sending GET request using HttpClient method..." << std::endl;
    std::future<kurlyk::HttpResponsePtr> future_response = client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers());

    kurlyk::HttpResponsePtr response = future_response.get();
    KURLYK_PRINT
        << "Response from HttpClient method:" << std::endl
        << "ready: " << response->ready << std::endl
        << "content: " << response->content << std::endl
        << "error_code: " << response->error_code << std::endl
        << "status_code: " << response->status_code << std::endl
        << "----------------------------------------" << std::endl;

    KURLYK_PRINT << "Sending GET request using standalone function..." << std::endl;
    std::future<kurlyk::HttpResponsePtr> future_response_func = kurlyk::http_get("https://httpbin.org/ip", kurlyk::QueryParams(), kurlyk::Headers());

    kurlyk::HttpResponsePtr response_func = future_response_func.get();
    KURLYK_PRINT
        << "Response from standalone function:" << std::endl
        << "ready: " << response_func->ready << std::endl
        << "content: " << response_func->content << std::endl
        << "error_code: " << response_func->error_code << std::endl
        << "status_code: " << response_func->status_code << std::endl
        << "----------------------------------------" << std::endl;

    std::system("pause");

    kurlyk::deinit();
    return 0;
}
