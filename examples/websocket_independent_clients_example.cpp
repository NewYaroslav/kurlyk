#include <kurlyk.hpp>
#include <thread>
#include <chrono>

void test_connect_disconnect(int n) {
    for (int i = 0; i < n; ++i) {
        KURLYK_PRINT << "Iteration " << i + 1 << " of " << n << std::endl;

        // Первый клиент: подключается, отправляет сообщение и отключается, затем повторяет.
        std::thread client1_thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            kurlyk::WebSocketClient client1("wss://echo-websocket.fly.dev/");
            const long rate_limit_id = client1.add_rate_limit_rps(2);
            client1.event_handler() = [rate_limit_id](std::unique_ptr<kurlyk::WebSocketEventData> event) {
                static int counter = 0;
                switch (event->event_type) {
                    case kurlyk::WebSocketEventType::Open:
                        KURLYK_PRINT << "Client 1: Connection opened" << std::endl;
                        event->sender->send_message("Client 1 says hello!", rate_limit_id);
                        break;
                    case kurlyk::WebSocketEventType::Message:
                        KURLYK_PRINT << "Client 1: Message received: " << event->message << std::endl;
                        event->sender->send_message("Client 1 message #" + std::to_string(counter++), rate_limit_id);
                        break;
                    case kurlyk::WebSocketEventType::Close:
                        KURLYK_PRINT << "Client 1: Connection closed with status: " << event->status_code << std::endl;
                        break;
                    case kurlyk::WebSocketEventType::Error:
                        KURLYK_PRINT << "Client 1: Error: " << event->error_code.message() << std::endl;
                        break;
                };
            };
            KURLYK_PRINT << "Client 1: Connecting..." << std::endl;
            client1.connect_and_wait();
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Дождаться приёма сообщения
            KURLYK_PRINT << "Client 1: Disconnecting..." << std::endl;
            client1.disconnect_and_wait();
            KURLYK_PRINT << "Client 1: End" << std::endl;
        });

        // Второй клиент: подключается, остаётся подключённым, периодически отправляет сообщения.
        std::thread client2_thread([]() {
            kurlyk::WebSocketClient client2("wss://echo-websocket.fly.dev/");
            const long rate_limit_id = client2.add_rate_limit_rps(2);
            client2.event_handler() = [rate_limit_id](std::unique_ptr<kurlyk::WebSocketEventData> event) {
                static int counter = 0;
                switch (event->event_type) {
                    case kurlyk::WebSocketEventType::Open:
                        KURLYK_PRINT << "Client 2: Connection opened" << std::endl;
                        event->sender->send_message("Client 2 says hello!", rate_limit_id);
                        break;
                    case kurlyk::WebSocketEventType::Message:
                        KURLYK_PRINT << "Client 2: Message received: " << event->message << std::endl;
                        event->sender->send_message("Client 2 message #" + std::to_string(counter++), rate_limit_id);
                        break;
                    case kurlyk::WebSocketEventType::Close:
                        KURLYK_PRINT << "Client 2: Connection closed with status: " << event->status_code << std::endl;
                        break;
                    case kurlyk::WebSocketEventType::Error:
                        KURLYK_PRINT << "Client 2: Error: " << event->error_code.message() << std::endl;
                        break;
                };
            };
            KURLYK_PRINT << "Client 1: Connecting..." << std::endl;
            client2.connect_and_wait();
            std::this_thread::sleep_for(std::chrono::seconds(10)); // Подключён 10 секунд
            KURLYK_PRINT << "Client 2: Disconnecting..." << std::endl;
            client2.disconnect_and_wait();
            KURLYK_PRINT << "Client 2: End" << std::endl;
        });

        // Подождать завершения работы потоков клиентов перед повторной итерацией
        client1_thread.join();
        client2_thread.join();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    int repeat_count = 3; // Количество повторов цикла подключения/отключения
    test_connect_disconnect(repeat_count);
    KURLYK_PRINT << "End" << std::endl;
    return 0;
}
