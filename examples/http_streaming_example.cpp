#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>

namespace {

void finish_once(std::promise<void>& promise) {
    try {
        promise.set_value();
    } catch (...) {
    }
}

} // namespace

int main() {
    kurlyk::init(true);

    std::promise<void> standalone_done_promise;
    auto standalone_done_future = standalone_done_promise.get_future();
    std::atomic<int> standalone_chunk_count(0);

    const uint64_t request_id = kurlyk::http_get(
        "http://httpbin.org/stream/5",
        kurlyk::QueryParams(),
        kurlyk::Headers(),
        true,
        [&standalone_done_promise, &standalone_chunk_count](kurlyk::HttpResponsePtr response) {
            if (!response) return;

            if (response->stream_chunk) {
                ++standalone_chunk_count;
                KURLYK_PRINT << "standalone chunk: " << response->content << std::endl;
                return;
            }
            if (!response->ready) return;

            KURLYK_PRINT
                << "standalone ready: " << std::boolalpha << response->ready << std::endl
                << "status_code: " << response->status_code << std::endl
                << "chunks: " << standalone_chunk_count.load() << std::endl
                << "error_code: " << response->error_code.message() << std::endl;

            finish_once(standalone_done_promise);
        });

    KURLYK_PRINT << "Standalone request id: " << request_id << std::endl;
    standalone_done_future.wait_for(std::chrono::seconds(30));

    std::promise<void> client_done_promise;
    auto client_done_future = client_done_promise.get_future();
    std::atomic<int> client_chunk_count(0);

    kurlyk::HttpClient client("http://httpbin.org");
    client.set_streaming(true);
    client.get("/stream/5", kurlyk::QueryParams(), kurlyk::Headers(),
        [&client_done_promise, &client_chunk_count](kurlyk::HttpResponsePtr response) {
            if (!response) return;

            if (response->stream_chunk) {
                ++client_chunk_count;
                KURLYK_PRINT << "client chunk: " << response->content << std::endl;
                return;
            }
            if (!response->ready) return;

            KURLYK_PRINT
                << "client ready: " << std::boolalpha << response->ready << std::endl
                << "status_code: " << response->status_code << std::endl
                << "chunks: " << client_chunk_count.load() << std::endl
                << "error_code: " << response->error_code.message() << std::endl;

            finish_once(client_done_promise);
        });

    client_done_future.wait_for(std::chrono::seconds(30));

    kurlyk::deinit();
    return 0;
}
