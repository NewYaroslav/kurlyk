#include <kurlyk.hpp>
#include <thread>
#include <chrono>

void test_connect_disconnect(int n) {
    for (int i = 0; i < n; ++i) {
        KURLYK_PRINT << "Iteration " << i + 1 << " of " << n << std::endl;

        // First client: connects and disconnects in quick succession, then exits scope
        {
            kurlyk::WebSocketClient client("wss://echo-websocket.fly.dev/");
            client.event_handler() = [](std::unique_ptr<kurlyk::WebSocketEventData> event) {
                switch (event->event_type) {
                    case kurlyk::WebSocketEventType::Open:
                        KURLYK_PRINT << "Client 1: Connection opened" << std::endl;
                        break;
                    case kurlyk::WebSocketEventType::Message:
                        KURLYK_PRINT << "Client 1: Message received: " << event->message << std::endl;
                        break;
                    case kurlyk::WebSocketEventType::Close:
                        KURLYK_PRINT << "Client 1: Connection closed: " << event->message
                                     << "; Status code: " << event->status_code << std::endl;
                        break;
                    case kurlyk::WebSocketEventType::Error:
                        KURLYK_PRINT << "Client 1: Error: " << event->error_code.message() << std::endl;
                        break;
                };
            };
            for (int j = 0; j < 100; ++j) {
                client.connect();
                client.disconnect();
            }
            KURLYK_PRINT << "Client 1: Connecting..." << std::endl;
            client.connect();
        } // Client 1 exits scope here

        // Second client: connects, stays connected for 10 seconds, then exits scope
        {
            kurlyk::WebSocketClient client("wss://echo-websocket.fly.dev/");
            const long rate_limit_id = client.add_rate_limit_rps(2);
            KURLYK_PRINT << "rate_limit_id " << rate_limit_id << std::endl;
            client.event_handler() = [rate_limit_id](std::unique_ptr<kurlyk::WebSocketEventData> event) {
                static int counter = 0;
                switch (event->event_type) {
                    case kurlyk::WebSocketEventType::Open:
                        KURLYK_PRINT << "Client 2: Connection opened" << std::endl;
                        // Send a message
                        event->sender->send_message("Hello, WebSocket! Counter: " + std::to_string(counter++), rate_limit_id);
                        break;
                    case kurlyk::WebSocketEventType::Message:
                        KURLYK_PRINT << "Client 2: Message received: " << event->message << std::endl;
                        // Send a response
                        event->sender->send_message("Hello again! Counter: " + std::to_string(counter++), rate_limit_id);
                        break;
                    case kurlyk::WebSocketEventType::Close:
                        KURLYK_PRINT << "Client 2: Connection closed: " << event->message
                                     << "; Status code: " << event->status_code << std::endl;
                        break;
                    case kurlyk::WebSocketEventType::Error:
                        KURLYK_PRINT << "Client 2: Error: " << event->error_code.message() << std::endl;
                        break;
                };
            };
            for (int j = 0; j < 100; ++j) {
                client.connect();
                client.disconnect();
            }
            KURLYK_PRINT << "Client 2: Connecting..." << std::endl;
            client.connect();
            std::this_thread::sleep_for(std::chrono::seconds(60));
            KURLYK_PRINT << "Client 2: Disconnecting..." << std::endl;
            client.disconnect_and_wait();
        } // Client 2 exits scope after 10 seconds
    }
}

int main() {
    int repeat_count = 5; // Number of times to repeat the connect/disconnect cycle
    test_connect_disconnect(repeat_count);
    return 0;
}

