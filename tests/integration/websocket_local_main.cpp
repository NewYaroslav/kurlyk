#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <server_ws.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

namespace {

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

} // namespace

int main() {
    WsServer server;
    server.config.port = 0;
    server.config.thread_pool_size = 1;

    std::atomic<int> server_message_count(0);
    std::promise<unsigned short> port_promise;
    auto port_future = port_promise.get_future();

    auto& echo = server.endpoint["^/echo/?$"];
    echo.on_message = [&server_message_count](std::shared_ptr<WsServer::Connection> connection,
                                              std::shared_ptr<WsServer::InMessage> in_message) {
        ++server_message_count;
        connection->send(in_message->string());
    };
    echo.on_error = [](std::shared_ptr<WsServer::Connection>, const SimpleWeb::error_code& ec) {
        std::cerr << "Server error: " << ec.message() << std::endl;
        std::exit(1);
    };

    std::thread server_thread([&server, &port_promise]() {
        server.start([&port_promise](unsigned short port) {
            try {
                port_promise.set_value(port);
            } catch (...) {
            }
        });
    });

    const unsigned short port = port_future.get();

    std::promise<void> done_promise;
    auto done_future = done_promise.get_future();
    std::atomic<bool> done{false};
    std::vector<kurlyk::WebSocketEventType> events;
    std::string echoed_message;
    long open_status_code = 0;

    kurlyk::init(true);
    {
        kurlyk::WebSocketClient client("ws://127.0.0.1:" + std::to_string(port) + "/echo");
        client.on_event([&](std::unique_ptr<kurlyk::WebSocketEventData> event) {
            events.push_back(event->event_type);
            switch (event->event_type) {
            case kurlyk::WebSocketEventType::WS_OPEN:
                open_status_code = event->status_code;
                require(static_cast<bool>(event->sender), "WS_OPEN sender is null");
                require(event->sender->is_connected(), "WS_OPEN sender is not connected");
                require(event->sender->send_message("local-echo-check"),
                        "Failed to send message from WS_OPEN sender");
                break;
            case kurlyk::WebSocketEventType::WS_MESSAGE:
                echoed_message = event->message;
                require(static_cast<bool>(event->sender), "WS_MESSAGE sender is null");
                require(event->sender->send_close(1000, "done"),
                        "Failed to send close from WS_MESSAGE sender");
                break;
            case kurlyk::WebSocketEventType::WS_CLOSE:
                if (!done.exchange(true)) {
                    try {
                        done_promise.set_value();
                    } catch (...) {
                    }
                }
                break;
            case kurlyk::WebSocketEventType::WS_ERROR:
                std::cerr << "Client error: " << event->error_code.message() << std::endl;
                std::exit(1);
            }
        });

        require(client.connect_and_wait(), "WebSocket connection failed");

        const auto wait_status = done_future.wait_for(std::chrono::seconds(10));
        require(wait_status == std::future_status::ready, "Timed out waiting for websocket close event");
    }
    kurlyk::deinit();

    server.stop();
    server_thread.join();

    require(server_message_count.load() == 1, "Local echo server did not receive exactly one message");
    require(events.size() == 3, "Unexpected websocket event count");
    require(events[0] == kurlyk::WebSocketEventType::WS_OPEN, "First websocket event is not WS_OPEN");
    require(events[1] == kurlyk::WebSocketEventType::WS_MESSAGE, "Second websocket event is not WS_MESSAGE");
    require(events[2] == kurlyk::WebSocketEventType::WS_CLOSE, "Third websocket event is not WS_CLOSE");
    require(open_status_code == 101, "Unexpected websocket open status code");
    require(echoed_message == "local-echo-check", "Unexpected echoed websocket message");

    std::cout << "Local websocket integration test passed" << std::endl;
    return 0;
}
