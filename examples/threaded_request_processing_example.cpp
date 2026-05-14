#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    kurlyk::init(false);

    std::atomic<bool> running(true);
    std::atomic<bool> completed(false);

    std::thread processing_thread([&running]() {
        while (running) {
            kurlyk::process();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    {
        kurlyk::HttpClient client("https://httpbin.org");
        client.set_user_agent("KurlykClient/1.0");
        client.set_timeout(10);
        client.set_retry_attempts(3, 1000);

        KURLYK_PRINT << "Sending GET request..." << std::endl;
        client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
            [&completed](const kurlyk::HttpResponsePtr response) {
                if (!response || !response->ready) return;

                KURLYK_PRINT
                    << "GET Response Content: " << response->content << std::endl
                    << "Status Code: " << response->status_code << std::endl
                    << "Error: " << response->error_code.message() << std::endl;
                completed = true;
            });

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
        while (!completed && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    running = false;
    processing_thread.join();
    kurlyk::deinit();

    KURLYK_PRINT << "Request processing completed. Exiting program." << std::endl;
    return 0;
}
