# kurlyk
![logo](docs/logo-mini.png)

**C++ library for easy networking**

[Do you speak English?](README.md)

## Описание

**kurlyk** — это ещё одна библиотека, реализующая HTTP и WebSocket клиенты для C++. Построена как обертка над `curl` и `Simple-WebSocket-Server`, предоставляя упрощённый интерфейс для работы с HTTP и WebSocket в C++ приложениях. Она поддерживает асинхронное выполнение HTTP-запросов с ограничением скорости и повторными попытками, а также работу с WebSocket-соединениями.

Если вам не подошли другие библиотеки, такие как *easyhttp-cpp, curl_request, curlpp-async, curlwrapper, curl-Easy-cpp, curlpp11, easycurl, curl-cpp-wrapper…* возможно, стоит попробовать `kurlyk`.

### Особенности

- Асинхронное выполнение HTTP и WebSocket запросов
- Поддержка ограничения скорости для предотвращения перегрузки сети
- Автоматическое переподключение с настраиваемыми параметрами
- Простота использования через интуитивно понятный интерфейс классов
- Ориентация на использование в небольших приложениях
- Поддержка С++11

## Примеры использования

Примеры находятся в папке `examples`. Ниже приведены основные примеры использования библиотеки.

### Пример использования WebSocket клиента

Этот пример показывает, как подключиться к WebSocket серверу, отправить сообщение и обработать различные события (открытие соединения, получение сообщения, закрытие соединения и ошибки).:

```cpp
#include <kurlyk.hpp>

int main() {
    // Создаём клиент WebSocket с указанным URL сервера
    kurlyk::WebSocketClient client("wss://echo-websocket.fly.dev/");

    // Настраиваем обработчик событий WebSocket
    client.on_event([](std::unique_ptr<kurlyk::WebSocketEventData> event) {
        switch (event->event_type) {
            case kurlyk::WebSocketEventType::WS_OPEN:
                KURLYK_PRINT << "Соединение установлено" << std::endl;

                // Выводим HTTP версию и заголовки
                KURLYK_PRINT << "HTTP версия: " << event->sender->get_http_version() << std::endl;
                KURLYK_PRINT << "Заголовки:" << std::endl;
                for (const auto& header : event->sender->get_headers()) {
                    KURLYK_PRINT << header.first << ": " << header.second << std::endl;
                }

                // Отправляем сообщение
                event->sender->send_message("Привет, WebSocket!", 0, [](const std::error_code& ec) {
                    if (ec) {
                        KURLYK_PRINT << "Ошибка отправки сообщения: " << ec.message() << std::endl;
                    } else {
                        KURLYK_PRINT << "Сообщение успешно отправлено" << std::endl;
                    }
                });
                break;

            case kurlyk::WebSocketEventType::WS_MESSAGE:
                KURLYK_PRINT << "Получено сообщение: " << event->message << std::endl;

                // Отправляем ответ
                event->sender->send_message("Привет снова!");
                break;

            case kurlyk::WebSocketEventType::WS_CLOSE:
                KURLYK_PRINT << "Соединение закрыто: " << event->message
                             << "; Код статуса: " << event->status_code << std::endl;
                break;

            case kurlyk::WebSocketEventType::WS_ERROR:
                KURLYK_PRINT << "Ошибка: " << event->error_code.message() << std::endl;
                break;
        };
    });

    // Подключаемся к серверу
    KURLYK_PRINT << "Подключение..." << std::endl;
    client.connect();

    // Ожидаем некоторое время для приёма сообщений
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Отправляем последнее сообщение через класс клиента
    client.send_message("Пока!");

    // Отключаемся от сервера
    KURLYK_PRINT << "Отключение..." << std::endl;
    client.disconnect_and_wait();

    KURLYK_PRINT << "Конец работы" << std::endl;
    return 0;
}
```

### Примеры использования HTTP-клиента

Эти примеры показывают, как использовать HTTP-клиент библиотеки kurlyk для выполнения различных запросов и обработки ответов.

#### Пример 1: Выполнение GET и POST запросов с обработчиком ответов

```cpp
#include <kurlyk.hpp>
#include <iostream>

// Функция для вывода информации об ответе
void print_response(const kurlyk::HttpResponsePtr& response) {
    KURLYK_PRINT
        << "ready: " << response->ready << std::endl
        << "response: " << response->content << std::endl
        << "error_code: " << response->error_code << std::endl
        << "status_code: " << response->status_code << std::endl
        << "----------------------------------------" << std::endl;
}

int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    // Отправка GET запроса с обработчиком
    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    // Отправка POST запроса с обработчиком
    client.post("/post", kurlyk::QueryParams(), {{"Content-Type", "application/json"}}, "{\"text\":\"Sample POST Content\"}",
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    kurlyk::deinit();
    return 0;
}
```

#### Пример 2: Выполнение GET и POST запросов с std::future

```cpp
#include <kurlyk.hpp>
#include <iostream>

int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    // Асинхронный GET запрос
    auto future_response = client.get("/get", kurlyk::QueryParams{{"param", "value"}}, kurlyk::Headers());
    kurlyk::HttpResponsePtr response = future_response.get();
    print_response(response);

    // Асинхронный POST запрос
    auto future_post = client.post("/post", kurlyk::QueryParams(), kurlyk::Headers{{"Header", "Value"}}, "Async POST Content");
    kurlyk::HttpResponsePtr post_response = future_post.get();
    print_response(post_response);

    kurlyk::deinit();
    return 0;
}
```

#### Пример 3: Настройка прокси и отправка GET запроса

```cpp
#include <kurlyk.hpp>
#include <iostream>

int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    // Устанавливаем параметры прокси
    client.set_proxy("127.0.0.1", 8080, "username", "password", kurlyk::ProxyType::HTTP);

    // Отправка GET запроса через прокси
    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    kurlyk::deinit();
    return 0;
}
```

#### Пример 4: GET запрос с использованием http_get функции

```cpp
#include <kurlyk.hpp>
#include <iostream>

int main() {
    kurlyk::init(true);

    // Асинхронный GET запрос с использованием standalone функции
    uint64_t request_id = kurlyk::http_get("https://httpbin.org/ip", kurlyk::QueryParams(), kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

	std::system("pause");
    kurlyk::cancel_request_by_id(request_id).wait();
    kurlyk::deinit();
    return 0;
}
```

## Зависимости и установка

Для работы библиотеки **kurlyk** в среде MinGW потребуются следующие зависимости:

1. Для WebSocket:

    - [Simple-WebSocket-Server](https://gitlab.com/eidheim/Simple-WebSocket-Server)
    - Boost.Asio или [standalone Asio](https://github.com/chriskohlhoff/asio/tree/master)
    - [OpenSSL](https://slproweb.com/products/Win32OpenSSL.html) (*LTS версия Win64 OpenSSL v3.0.15*)
    
2. Для HTTP:
    - [libcurl](https://curl.se/windows/)

Все зависимости также добавлены в проект в виде субмодулей, находящихся в папке `libs`.

### Подключение OpenSSL

1. Добавьте в проект пути к OpenSSL (пример для версии *3.4.0*):

```
OpenSSL-Win64/include
OpenSSL-Win64/lib/VC/x64/MD
OpenSSL-Win64/bin
```

2. Подключите библиотеки OpenSSL из папки `lib/VC/x64/MD`:

```
capi.lib
dasync.lib
libcrypto.lib
libssl.lib
openssl.lib
ossltest.lib
padlock.lib
```

### Подключение Standalone Asio

1. Добавьте в проект путь к asio (пример для [репозитория Asio](https://github.com/chriskohlhoff/asio/tree/master)):

```
asio/asio/include
```

2. Задайте макрос `ASIO_STANDALONE` в параметрах проекта или перед подключением `kurlyk.hpp`:

```cpp
#define ASIO_STANDALONE
#include <kurlyk.hpp>
```

> **Примечание:** Для Boost.Asio указывать макрос `ASIO_STANDALONE` не нужно.

### Подключение curl

1. Добавьте в проект пути для `curl` (пример для версии *8.11.0*):

```
curl-8.11.0_1-win64-mingw/bin
curl-8.11.0_1-win64-mingw/include
curl-8.11.0_1-win64-mingw/lib
```

2. Подключите библиотеки `curl` из папки `lib`:

```
libcurl.a
libcurl.dll.a
```

### Подключение Simple-WebSocket-Server

Добавьте в проект путь к заголовочным файлам библиотеки:

```
Simple-WebSocket-Server
```

### Подключение остальных зависимостей

Также добавьте следующие библиотеки в линкер:

```
ws2_32
wsock32
crypt32
```

### Подключение kurlyk

Добавьте в проект путь к заголовочным файлам библиотеки:

```
kurlyk/include
```

**kurlyk** — это header-only библиотека, поэтому достаточно просто подключить её через `#include <kurlyk.hpp>` и начать использовать.

## Конфигурационные макросы

Перед подключением `kurlyk.hpp` можно определить следующие макросы для тонкой
настройки библиотеки:

- `KURLYK_AUTO_INIT` (по умолчанию `1`) — автоматическая регистрация менеджеров
  во время статической инициализации.
- `KURLYK_AUTO_INIT_USE_ASYNC` (по умолчанию `1`) — при включённом auto init
  запускает сетевой поток в фоне. Установите `0`, если требуется выполнять
  обработку вручную.
- `KURLYK_HTTP_SUPPORT` и `KURLYK_WEBSOCKET_SUPPORT` (по умолчанию `1`) —
  включают или отключают соответствующие подсистемы.
- `KURLYK_ENABLE_JSON` (по умолчанию `0`) — добавляет вспомогательные функции
  для JSON-сериализации некоторых типов.

## Документация

В процессе написания. 

## Лицензия

Эта библиотека распространяется под лицензией MIT. Подробности смотрите в файле [LICENSE](LICENSE) в репозитории.

## Поддержка

Если у вас возникли вопросы или проблемы при использовании библиотеки, вы можете обратиться к документации или оставить вопрос в разделе Issues на GitHub.


Одним словом, **kurlyk!**

![logo](docs/logo-mini-end.png)

