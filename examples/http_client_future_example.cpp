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
#   if __cplusplus >= 201703L
    auto [request_id, future] = kurlyk::http_get("https://httpbin.org/ip", kurlyk::QueryParams(), kurlyk::Headers());
#   else
    auto func_result = kurlyk::http_get("https://httpbin.org/ip", kurlyk::QueryParams(), kurlyk::Headers());
    uint64_t request_id = func_result.first;
    auto future = std::move(func_result.second);
#   endif

    kurlyk::HttpResponsePtr response_func = future.get();
    KURLYK_PRINT
        << "Response from standalone function:" << std::endl
        << "Request ID: " << request_id << std::endl
        << "Ready: " << std::boolalpha << response_func->ready << std::endl
        << "Content: " << response_func->content << std::endl
        << "Error Code: " << response_func->error_code.message() << std::endl
        << "Status Code: " << response_func->status_code << std::endl
        << "----------------------------------------" << std::endl;

    std::cin.get();

    kurlyk::deinit();
    return 0;
}
