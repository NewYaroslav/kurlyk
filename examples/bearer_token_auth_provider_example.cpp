#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string load_api_key(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open the API key file.");
    }
    std::string api_key;
    std::getline(file, api_key);
    if (api_key.empty()) {
        throw std::runtime_error("API key is missing.");
    }
    return api_key;
}

int main() {
    kurlyk::init(true);
    try {
        const std::string api_key = load_api_key("openai_api_key.txt");

        kurlyk::Headers headers = { {"Content-Type", "application/json"} };

        json request_body = {
            {"model", "gpt-3.5-turbo"},
            {"messages", {
                {{"role", "user"}, {"content", u8"Hello, ChatGPT!"}}
            }},
            {"max_tokens", 50}
        };

        std::string host = "https://neuroapi.host";
        kurlyk::HttpClient client;
        client.set_host(host);
        client.set_rate_limit_rpm(3);
        client.set_retry_attempts(1, 5000);
        client.set_verbose(true);

        kurlyk::http::auth::BearerTokenAuthProvider auth_provider(api_key);
        auth_provider.authorize(headers);

        KURLYK_PRINT << "Request body: " << request_body.dump(4) << std::endl;

        auto future = client.post("/v1/chat/completions", {}, headers, request_body.dump());

        auto response = future.get();
        if (response->ready && response->status_code == 200) {
            KURLYK_PRINT << "Response: " << response->content << std::endl;
        } else {
            KURLYK_PRINT << "Error: " << response->status_code
                         << " - " << response->error_code.message() << std::endl;
            KURLYK_PRINT << "Response: " << response->content << std::endl;
        }

    } catch (const std::exception& e) {
        KURLYK_PRINT << "Error: " << e.what() << std::endl;
    }

    kurlyk::deinit();
    return 0;
}
