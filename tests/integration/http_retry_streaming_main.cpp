#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <server_http.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

std::string make_response(long status, const std::string& reason, const std::string& body) {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << ' ' << reason << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Content-Type: text/plain\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return out.str();
}

struct CallbackStats {
    std::mutex mutex;
    int ready_count = 0;
    int intermediate_count = 0;
    int stream_chunk_count = 0;
    long final_status = 0;
    long final_retry_attempt = 0;
    std::string final_content;
    std::vector<long> intermediate_statuses;
    std::vector<std::string> chunks;
};

} // namespace

int main() {
    HttpServer server;
    server.config.address = "127.0.0.1";
    server.config.port = 0;
    server.config.thread_pool_size = 1;

    std::atomic<int> unstable_hits(0);
    std::atomic<int> stream_partial_hits(0);

    server.resource["^/unstable$"]["GET"] = [&unstable_hits](std::shared_ptr<HttpServer::Response> response,
                                                              std::shared_ptr<HttpServer::Request>) {
        const int hit = ++unstable_hits;
        if (hit < 3) {
            *response << make_response(500, "Internal Server Error", "retry-me");
            return;
        }
        *response << make_response(200, "OK", "retry-ok");
    };

    server.resource["^/always-fail$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                   std::shared_ptr<HttpServer::Request>) {
        *response << make_response(500, "Internal Server Error", "still-failing");
    };

    server.resource["^/stream$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                              std::shared_ptr<HttpServer::Request>) {
        *response << make_response(200, "OK", "stream-body");
    };

    // Sends fewer bytes than declared. libcurl should report a transfer error after
    // at least one body chunk. kurlyk must not retry after streaming data was emitted.
    server.resource["^/stream-partial$"]["GET"] = [&stream_partial_hits](std::shared_ptr<HttpServer::Response> response,
                                                                          std::shared_ptr<HttpServer::Request>) {
        ++stream_partial_hits;
        *response << "HTTP/1.1 200 OK\r\n"
                  << "Content-Length: 64\r\n"
                  << "Content-Type: text/plain\r\n"
                  << "Connection: close\r\n\r\n"
                  << "partial-body";
    };

    const unsigned short port = server.bind();
    std::thread server_thread([&server]() {
        server.accept_and_run();
    });
    const std::string host = "http://127.0.0.1:" + std::to_string(port);

    kurlyk::init(true);
    {
        kurlyk::HttpClient retry_client(host);
        retry_client.set_retry_attempts(3, 0);

        CallbackStats retry_stats;
        std::promise<void> retry_done;
        auto retry_future = retry_done.get_future();
        retry_client.get("/unstable", kurlyk::QueryParams(), kurlyk::Headers(),
            [&retry_stats, &retry_done](kurlyk::HttpResponsePtr response) {
                require(static_cast<bool>(response), "Retry callback received null response");
                std::lock_guard<std::mutex> lock(retry_stats.mutex);
                if (response->ready) {
                    ++retry_stats.ready_count;
                    retry_stats.final_status = response->status_code;
                    retry_stats.final_retry_attempt = response->retry_attempt;
                    retry_stats.final_content = response->content;
                    retry_done.set_value();
                    return;
                }
                ++retry_stats.intermediate_count;
                retry_stats.intermediate_statuses.push_back(response->status_code);
            });
        require(retry_future.wait_for(std::chrono::seconds(10)) == std::future_status::ready,
                "Timed out waiting for retry success response");
        require(unstable_hits.load() == 3, "Retry test should perform exactly three attempts");
        require(retry_stats.intermediate_count == 2, "Retry test should emit two intermediate callbacks");
        require(retry_stats.ready_count == 1, "Retry test should emit one final callback");
        require(retry_stats.final_status == 200, "Retry test final status should be 200");
        require(retry_stats.final_retry_attempt == 3, "Retry test final retry_attempt should be 3");
        require(retry_stats.final_content == "retry-ok", "Retry test final body mismatch");

        auto failed = retry_client.get("/always-fail", kurlyk::QueryParams(), kurlyk::Headers()).get();
        require(failed && failed->ready, "Always-fail retry request returned no final response");
        require(failed->status_code == 500, "Always-fail retry request should end with HTTP 500");
        require(failed->retry_attempt == 3, "Always-fail retry request should exhaust three attempts");
        require(static_cast<bool>(failed->error_code), "Always-fail retry request should set error_code");

        kurlyk::HttpClient streaming_client(host);
        streaming_client.set_streaming(true);

        CallbackStats stream_stats;
        std::promise<void> stream_done;
        auto stream_future = stream_done.get_future();
        streaming_client.get("/stream", kurlyk::QueryParams(), kurlyk::Headers(),
            [&stream_stats, &stream_done](kurlyk::HttpResponsePtr response) {
                require(static_cast<bool>(response), "Streaming callback received null response");
                std::lock_guard<std::mutex> lock(stream_stats.mutex);
                if (response->stream_chunk) {
                    ++stream_stats.stream_chunk_count;
                    stream_stats.chunks.push_back(response->content);
                    require(!response->ready, "Streaming chunk must not be marked ready");
                    return;
                }
                if (response->ready) {
                    ++stream_stats.ready_count;
                    stream_stats.final_status = response->status_code;
                    stream_stats.final_content = response->content;
                    stream_done.set_value();
                }
            });
        require(stream_future.wait_for(std::chrono::seconds(10)) == std::future_status::ready,
                "Timed out waiting for streaming final response");
        require(stream_stats.stream_chunk_count >= 1, "Streaming request should emit at least one chunk callback");
        require(stream_stats.ready_count == 1, "Streaming request should emit one final callback");
        require(stream_stats.final_status == 200, "Streaming final status should be 200");
        require(stream_stats.final_content == "stream-body", "Streaming final body mismatch");

        kurlyk::HttpClient partial_client(host);
        partial_client.set_streaming(true);
        partial_client.set_retry_attempts(3, 0);

        CallbackStats partial_stats;
        std::promise<void> partial_done;
        auto partial_future = partial_done.get_future();
        partial_client.get("/stream-partial", kurlyk::QueryParams(), kurlyk::Headers(),
            [&partial_stats, &partial_done](kurlyk::HttpResponsePtr response) {
                require(static_cast<bool>(response), "Partial streaming callback received null response");
                std::lock_guard<std::mutex> lock(partial_stats.mutex);
                if (response->stream_chunk) {
                    ++partial_stats.stream_chunk_count;
                    return;
                }
                if (response->ready) {
                    ++partial_stats.ready_count;
                    partial_stats.final_status = response->status_code;
                    partial_stats.final_retry_attempt = response->retry_attempt;
                    partial_stats.final_content = response->content;
                    partial_done.set_value();
                }
            });
        require(partial_future.wait_for(std::chrono::seconds(10)) == std::future_status::ready,
                "Timed out waiting for partial streaming final response");
        require(stream_partial_hits.load() == 1,
                "Partial streaming request must not retry after emitting a body chunk");
        require(partial_stats.stream_chunk_count >= 1,
                "Partial streaming request should emit at least one chunk callback");
        require(partial_stats.ready_count == 1, "Partial streaming request should emit one final callback");
        require(partial_stats.final_retry_attempt == 1,
                "Partial streaming final retry_attempt should remain 1 when retry is suppressed");
        require(static_cast<bool>(partial_stats.final_status),
                "Partial streaming final response should contain a status code");
    }
    kurlyk::deinit();

    server.stop();
    server_thread.join();

    std::cout << "HTTP retry and streaming integration test passed" << std::endl;
    return 0;
}
