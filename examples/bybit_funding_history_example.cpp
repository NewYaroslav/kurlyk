#include <kurlyk.hpp>
#include <iostream>

// Thread-safe logging of response details
void print_response(const kurlyk::HttpResponsePtr& response) {
    KURLYK_PRINT << "Headers:" << std::endl;
    for (const auto& header : response->headers) {
        KURLYK_PRINT << header.first << ": " << header.second << std::endl;
    }

    KURLYK_PRINT
        << "Request complete:" << std::endl
        << "Ready: " << response->ready << std::endl
        << "Content: " << response->content << std::endl
        << "Error Code: " << response->error_code << std::endl
        << "Status Code: " << response->status_code << std::endl
        << "----------------------------------------" << std::endl;
}

int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://api.bybit.com");
    client.set_rate_limit_rps(10); // According to Bybit's API documentation, the rate limit for this endpoint is 10 requests per second.

    // Direct URL request
    KURLYK_PRINT << "Sending request with direct URL..." << std::endl;
    client.get("/v5/market/funding/history?category=linear&symbol=ETHPERP&limit=1", kurlyk::QueryParams(), kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    // Request with QueryParams
    KURLYK_PRINT << "Sending request using QueryParams..." << std::endl;
    kurlyk::QueryParams params = {
        {"category", "linear"},
        {"symbol", "ETHPERP"},
        {"limit", "1"}
    };

    client.get("/v5/market/funding/history", params, kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    KURLYK_PRINT << "Press Enter to exit..." << std::endl;
    std::cin.get();
    client.cancel_requests();
    kurlyk::deinit();
    return 0;
}
