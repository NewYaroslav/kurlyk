#include <kurlyk.hpp>

int main() {
    kurlyk::WebSocketClient client("wss://echo-websocket.fly.dev/");

    client.on_event([](std::unique_ptr<kurlyk::WebSocketEventData> event) {
        switch (event->event_type) {
            case kurlyk::WebSocketEventType::WS_OPEN:
                KURLYK_PRINT << "Connection opened" << std::endl;

                // Output HTTP version
                KURLYK_PRINT << "HTTP Version: " << event->sender->get_http_version() << std::endl;

                // Output headers
                KURLYK_PRINT << "Headers:" << std::endl;
                for (const auto& header : event->sender->get_headers()) {
                    KURLYK_PRINT << header.first << ": " << header.second << std::endl;
                }

                // Send a message
                event->sender->send_message("Hello, WebSocket!", 0, [](const std::error_code& ec) {
                    if (ec) {
                        KURLYK_PRINT << "Failed to send message: " << ec.message() << std::endl;
                    } else {
                        KURLYK_PRINT << "Message sent successfully" << std::endl;
                    }
                });
                break;

            case kurlyk::WebSocketEventType::WS_MESSAGE:
                KURLYK_PRINT << "Message received: " << event->message << std::endl;

                // Send a response
                event->sender->send_message("Hello again!", 0, [](const std::error_code& ec) {
                    if (ec) {
                        KURLYK_PRINT << "Failed to send message: " << ec.message() << std::endl;
                    } else {
                        KURLYK_PRINT << "Message sent successfully" << std::endl;
                    }
                });
                break;

            case kurlyk::WebSocketEventType::WS_CLOSE:
                KURLYK_PRINT << "Connection closed: " << event->message
                             << "; Status code: " << event->status_code << std::endl;
                break;

            case kurlyk::WebSocketEventType::WS_ERROR:
                KURLYK_PRINT << "Error: " << event->error_code.message() << std::endl;
                break;
        };
    });

    KURLYK_PRINT << "Connecting..." << std::endl;
    client.connect();

    // Wait to receive responses
    std::this_thread::sleep_for(std::chrono::seconds(10));

    KURLYK_PRINT << "Disconnecting..." << std::endl;
    client.disconnect_and_wait();

    KURLYK_PRINT << "End" << std::endl;
    return 0;
}
