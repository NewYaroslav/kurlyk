#include <iostream>
#include <kurlyk.hpp>
#include <thread>
#include <atomic>

int main() {
    // Initialize the library in synchronous mode (own-thread handling)
    kurlyk::init(false);

    std::atomic<bool> running{true}; // Atomic flag to control the processing loop

    // Start a separate thread to process requests
    std::thread processing_thread([&running](){
        while (running) {
            kurlyk::process(); // Processes pending requests in synchronous mode
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Throttle to avoid busy-waiting
        }
        // Reset and deinitialize the library
        kurlyk::reset();
    });

    // Set up an HTTP client and configure requests
    kurlyk::HttpClient client("https://httpbin.org");
    client.set_user_agent("KurlykClient/1.0");
    client.set_timeout(10); // Timeout for requests
    client.set_retry_attempts(3, 1000); // Retry up to 3 times with 1s delay

    // Send a GET request
    KURLYK_PRINT << "Sending GET request..." << std::endl;
    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
        [&running](const kurlyk::HttpResponsePtr response) {
            KURLYK_PRINT
                << "GET Response Content: " << response->content << std::endl
                << "Status Code: " << response->status_code << std::endl;
            if (response->ready) running = false;
        });

    // Signal the processing thread to stop
    processing_thread.join(); // Wait for the processing thread to finish

    KURLYK_PRINT << "Request processing completed. Exiting program." << std::endl;
    return 0;
}
