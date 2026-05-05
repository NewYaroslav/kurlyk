#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>

int main() {
    kurlyk::init(true);

    std::promise<void> done_promise;
    auto done_future = done_promise.get_future();
    std::atomic<int> chunk_count(0);

    const uint64_t request_id = kurlyk::http_get(
        "http://httpbin.org/stream/5",
        kurlyk::QueryParams(),
        kurlyk::Headers(),
        true,
        [&done_promise, &chunk_count](kurlyk::HttpResponsePtr response) {
            if (!response) return;

            if (response->stream_chunk) {
                ++chunk_count;
                KURLYK_PRINT << "chunk: " << response->content << std::endl;
                return;
            }
            if (!response->ready) return;

            KURLYK_PRINT
                << "ready: " << std::boolalpha << response->ready << std::endl
                << "status_code: " << response->status_code << std::endl
                << "chunks: " << chunk_count.load() << std::endl
                << "error_code: " << response->error_code.message() << std::endl;

            try {
                done_promise.set_value();
            } catch (...) {
            }
        });

    KURLYK_PRINT << "Request id: " << request_id << std::endl;
    done_future.wait_for(std::chrono::seconds(30));

    kurlyk::deinit();
    return 0;
}
