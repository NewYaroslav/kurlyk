#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

void background_process(std::atomic<bool>& stop) {
    while (!stop.load()) {
        kurlyk::process();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

struct ProcessorGuard {
    std::atomic<bool> stop{false};
    std::thread thread;

    ProcessorGuard() : thread([this]() { background_process(stop); }) {}
    ~ProcessorGuard() {
        stop.store(true);
        if (thread.joinable()) thread.join();
    }
};

} // namespace

int main() {
    kurlyk::init(false);

    // --- Test 1: wait_requests() waits for callback ---
    {
        ProcessorGuard pg;

        auto client = std::make_unique<kurlyk::HttpClient>("http://127.0.0.1");
        client->set_connect_timeout(1);
        client->set_timeout(1);
        std::atomic<int> callback_count{0};

        bool ok = client->get("/", kurlyk::QueryParams(), kurlyk::Headers(),
            [&](kurlyk::HttpResponsePtr response) {
                if (response && response->ready) {
                    ++callback_count;
                }
            });
        require(ok, "request should be accepted");

        client->wait_requests();
        require(callback_count.load() == 1, "wait_requests() must wait until callback is delivered");
        require(client->in_flight_requests() == 0, "client group must be idle after wait_requests()");

        client.reset();
    }

    // --- Test 2: wait_requests() waits only current client group ---
    {
        ProcessorGuard pg;

        auto client_a = std::make_unique<kurlyk::HttpClient>("http://127.0.0.1");
        client_a->set_connect_timeout(1);
        client_a->set_timeout(1);
        auto client_b = std::make_unique<kurlyk::HttpClient>("http://127.0.0.1");
        client_b->set_connect_timeout(1);
        client_b->set_timeout(1);
        std::atomic<int> callback_a{0};
        std::atomic<int> callback_b{0};

        bool ok_a = client_a->get("/a", kurlyk::QueryParams(), kurlyk::Headers(),
            [&](kurlyk::HttpResponsePtr response) {
                if (response && response->ready) ++callback_a;
            });
        bool ok_b = client_b->get("/b", kurlyk::QueryParams(), kurlyk::Headers(),
            [&](kurlyk::HttpResponsePtr response) {
                if (response && response->ready) ++callback_b;
            });
        require(ok_a, "client_a request should be accepted");
        require(ok_b, "client_b request should be accepted");

        client_a->wait_requests();
        require(callback_a.load() == 1, "client_a.wait_requests() must wait until client_a callback is delivered");
        require(client_a->in_flight_requests() == 0, "client_a group must be idle after wait_requests()");

        client_a.reset();
        client_b.reset();
    }

    // --- Test 3: wait_requests_for() timeout ---
    {
        auto client = std::make_unique<kurlyk::HttpClient>("http://127.0.0.1");
        client->set_connect_timeout(1);
        client->set_timeout(1);
        std::atomic<int> callback_count{0};

        bool ok = client->get("/", kurlyk::QueryParams(), kurlyk::Headers(),
            [&](kurlyk::HttpResponsePtr response) {
                if (response && response->ready) ++callback_count;
            });
        require(ok, "request should be accepted");

        // No background processor running: request is pending but never processed.
        bool done = client->wait_requests_for(std::chrono::milliseconds(10));
        require(!done, "wait_requests_for() should return false on timeout when no processor is running");

        ProcessorGuard pg;

        bool done_long = client->wait_requests_for(std::chrono::seconds(2));
        require(done_long, "wait_requests_for() should return true before long timeout when processor runs");
        require(callback_count.load() == 1, "callback must be delivered after wait_requests_for succeeds");

        client.reset();
    }

    // --- Test 4: per-client max_in_flight ---
    {
        ProcessorGuard pg;

        auto client = std::make_unique<kurlyk::HttpClient>("http://127.0.0.1");
        client->set_connect_timeout(1);
        client->set_timeout(1);
        client->set_max_in_flight(1);
        std::atomic<int> callback_count{0};

        bool first = client->get("/first", kurlyk::QueryParams(), kurlyk::Headers(),
            [&](kurlyk::HttpResponsePtr response) {
                if (response && response->ready) ++callback_count;
            });
        bool second = client->get("/second", kurlyk::QueryParams(), kurlyk::Headers(),
            [&](kurlyk::HttpResponsePtr response) {
                if (response && response->ready) ++callback_count;
            });

        require(first, "first request should be accepted");
        require(!second, "second request should be rejected by per-client in-flight limit");

        client->wait_requests();
        require(callback_count.load() == 1, "only first callback should be delivered");

        bool third = client->get("/third", kurlyk::QueryParams(), kurlyk::Headers(),
            [&](kurlyk::HttpResponsePtr response) {
                if (response && response->ready) ++callback_count;
            });
        require(third, "request after group idle should be accepted");

        client->wait_requests();
        require(callback_count.load() == 2, "third callback should be delivered");

        client.reset();
    }

    // --- Test 5: future-based API with max_in_flight ---
    {
        ProcessorGuard pg;

        auto client = std::make_unique<kurlyk::HttpClient>("http://127.0.0.1");
        client->set_connect_timeout(1);
        client->set_timeout(1);
        client->set_max_in_flight(1);

        auto f1 = client->get("/", kurlyk::QueryParams(), kurlyk::Headers());
        auto f2 = client->get("/", kurlyk::QueryParams(), kurlyk::Headers());

        auto rejected = f2.get();
        require(rejected && rejected->ready, "rejected future must be ready");
        require(rejected->error_code == kurlyk::utils::make_error_code(kurlyk::utils::ClientError::QueueLimitExceeded),
                "rejected future must carry QueueLimitExceeded error code");

        auto completed = f1.get();
        require(completed && completed->ready, "first future must complete");

        client.reset();
    }

    kurlyk::deinit();
    std::cout << "HttpClient wait_requests and max_in_flight integration test passed" << std::endl;
    return 0;
}
