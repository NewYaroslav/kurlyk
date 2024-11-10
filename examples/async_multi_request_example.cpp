#include <iostream>
#include <kurlyk.hpp>
#include <future>
#include <vector>
#include <thread>
#include <chrono>

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

void handle_async_responses(std::vector<std::future<kurlyk::HttpResponsePtr>>& futures) {
    try {
        for (auto& future : futures) {
            kurlyk::HttpResponsePtr response = future.get();
            print_response(response);
        }
    } catch(...) {
        KURLYK_PRINT << "Error." << std::endl;
        throw;
    }
    KURLYK_PRINT << "All async requests completed in separate thread." << std::endl;
}

int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    // Set client parameters
    client.set_user_agent("KurlykClient/1.0");
    client.set_timeout(2);              // Request timeout in seconds
    client.set_connect_timeout(2);      // Connection timeout in seconds
    client.set_retry_attempts(3, 1000); // 3 attempts with a delay of 1 second
    client.set_rate_limit(5, 1000);     // 5 requests per second limit

    // GET request with callback handler
    KURLYK_PRINT << "Sending GET request with callback..." << std::endl;
    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
           print_response(response);
        });

    // POST request with callback handler
    KURLYK_PRINT << "Sending POST request with callback..." << std::endl;
    kurlyk::Headers post_headers;
    post_headers.emplace("Custom-Header", "HeaderValue");

    client.post("/post", kurlyk::QueryParams(), {{"Content-Type", "application/json"}}, "{\"text\":\"Sample POST Content\"}",
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    // Asynchronous GET requests using std::future
    KURLYK_PRINT << "Sending multiple async GET requests with std::future..." << std::endl;
    std::vector<std::future<kurlyk::HttpResponsePtr>> futures;
    for (int i = 0; i < 3; ++i) {
        futures.push_back(client.get("/get", kurlyk::QueryParams{{"param", std::to_string(i)}}, kurlyk::Headers()));
    }
    // Asynchronous POST request using std::future
    KURLYK_PRINT << "Sending async POST request with std::future..." << std::endl;
    auto future_post = client.post("/post", kurlyk::QueryParams(), kurlyk::Headers{{"Async-Header", "AsyncValue"}}, "Async POST Content");
    futures.push_back(std::move(future_post));

    // Start a separate thread to wait for all async responses
    std::thread async_thread(handle_async_responses, std::ref(futures));
    async_thread.join(); // Wait for the async thread to complete

    //std::system("pause");
    KURLYK_PRINT << "All requests completed." << std::endl;

    kurlyk::deinit();
    return 0;
}
