#include <kurlyk.hpp>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>  // For working with JSON (make sure to include the appropriate library)

using json = nlohmann::json;

// Function to load the API key and organization ID from a file
std::pair<std::string, std::string> load_api_credentials(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open the API credentials file.");
    }

    std::string api_key, organization;
    std::getline(file, api_key);      // Read the first line as API key
    std::getline(file, organization); // Read the second line as organization ID

    if (api_key.empty() || organization.empty()) {
        throw std::runtime_error("API key or organization ID is missing in the file.");
    }

    return {api_key, organization};
}

int main() {
    kurlyk::init(true); // Initialize the kurlyk library for async support
    try {
        // Load the API key and organization ID from the file
        auto [api_key, organization] = load_api_credentials("openai_api_key.txt");

        // Set headers for the request to ChatGPT
        kurlyk::Headers headers = {
            {"Authorization", "Bearer " + api_key},
            //{"OpenAI-Organization", organization},
            {"Content-Type", "application/json"}
        };

        // Create the request body in JSON format
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

        std::cout << "request_body: " << request_body.dump(4) << std::endl;

        // Send the POST request with JSON body
        auto future = client.post("/v1/chat/completions", {}, headers, request_body.dump());

        auto response = future.get();
        if (response->ready && response->status_code == 200) {
            std::cout << "Response from ChatGPT: " << response->content << std::endl;
        } else {
            std::cerr << "Error: " << response->status_code << "\n" << response->error_code.message() << std::endl;
            std::cout << "Response: " << response->content << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    kurlyk::deinit();  // Deinitialize the kurlyk library
    return 0;
}
