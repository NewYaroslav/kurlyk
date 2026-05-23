#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <iostream>

int main() {
    kurlyk::init(true);
    try {
        // Example: Google Gemini API key in a query parameter.
        // In a real application, load the key from a secure location.
        const std::string api_key = "your_gemini_api_key_here";

        kurlyk::Headers headers = { {"Content-Type", "application/json"} };

        kurlyk::HttpClient client;
        client.set_host("https://generativelanguage.googleapis.com");
        client.set_rate_limit_rpm(10);
        client.set_retry_attempts(1, 5000);

        // Use ApiKeyAuthProvider to append the key as a query parameter.
        kurlyk::http::auth::ApiKeyAuthProvider auth_provider(
            "key", api_key, kurlyk::http::auth::ApiKeyPlacement::QUERY);

        kurlyk::HttpRequest request;
        request.method = "GET";
        request.url = "/v1beta/models/gemini-pro";
        request.headers = headers;

        auth_provider.authorize(request);

        // After authorize, the URL should contain ?key=...
        KURLYK_PRINT << "Authorized URL: " << request.url << std::endl;

        auto future = client.get(request.url, {}, request.headers);
        auto response = future.get();

        if (response->ready && response->status_code == 200) {
            KURLYK_PRINT << "Response: " << response->content << std::endl;
        } else {
            KURLYK_PRINT << "Error: " << response->status_code
                         << " - " << response->error_code.message() << std::endl;
        }

    } catch (const std::exception& e) {
        KURLYK_PRINT << "Error: " << e.what() << std::endl;
    }

    kurlyk::deinit();
    return 0;
}
