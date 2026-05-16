#pragma once
#ifndef KURLYK_TESTS_LOCAL_HTTP_SERVER_HPP_INCLUDED
#define KURLYK_TESTS_LOCAL_HTTP_SERVER_HPP_INCLUDED

#include <asio.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

namespace kurlyk_tests {

struct LocalHttpRequest {
    std::string method;
    std::string target;
    std::string path;
    std::string query;
    std::map<std::string, std::string> headers;
    std::string body;
};

inline std::string make_http_response(long status, const std::string& reason, const std::string& body) {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << ' ' << reason << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Content-Type: text/plain\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return out.str();
}

inline std::string make_http_head_response(long status, const std::string& reason, std::size_t content_length) {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << ' ' << reason << "\r\n"
        << "Content-Length: " << content_length << "\r\n"
        << "Content-Type: text/plain\r\n"
        << "Connection: close\r\n\r\n";
    return out.str();
}

class LocalHttpServer {
public:
    using Handler = std::function<std::string(const LocalHttpRequest&)>;

    explicit LocalHttpServer(Handler handler)
        : m_handler(std::move(handler)),
          m_acceptor(m_io, asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), 0)) {
    }

    ~LocalHttpServer() {
        stop();
    }

    LocalHttpServer(const LocalHttpServer&) = delete;
    LocalHttpServer& operator=(const LocalHttpServer&) = delete;

    void start() {
        m_running = true;
        m_thread = std::thread([this]() { run(); });
    }

    void stop() {
        if (!m_running.exchange(false)) {
            return;
        }
        asio::error_code ignored;
        m_acceptor.close(ignored);
        m_io.stop();
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    unsigned short port() const {
        return m_acceptor.local_endpoint().port();
    }

    std::string host() const {
        return "http://127.0.0.1:" + std::to_string(port());
    }

private:
    Handler m_handler;
    asio::io_context m_io;
    asio::ip::tcp::acceptor m_acceptor;
    std::atomic<bool> m_running{false};
    std::thread m_thread;

    void run() {
        while (m_running) {
            asio::error_code ec;
            asio::ip::tcp::socket socket(m_io);
            m_acceptor.accept(socket, ec);
            if (ec) {
                if (m_running) {
                    std::cerr << "accept failed: " << ec.message() << std::endl;
                }
                continue;
            }
            handle_client(socket);
        }
    }

    void handle_client(asio::ip::tcp::socket& socket) {
        try {
            LocalHttpRequest request = read_request(socket);
            const std::string response = m_handler ? m_handler(request) : make_http_response(404, "Not Found", "not-found");
            asio::write(socket, asio::buffer(response));
        } catch (const std::exception& e) {
            std::cerr << "Local HTTP server error: " << e.what() << std::endl;
        }
        asio::error_code ignored;
        socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
        socket.close(ignored);
    }

    static std::string trim(std::string value) {
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.erase(value.begin());
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.pop_back();
        }
        return value;
    }

    static std::string lower_copy(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    static LocalHttpRequest parse_request_head(const std::string& head) {
        LocalHttpRequest request;
        std::istringstream stream(head);
        std::string line;

        if (!std::getline(stream, line)) {
            return request;
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::istringstream first_line(line);
        first_line >> request.method >> request.target;

        const std::size_t query_pos = request.target.find('?');
        if (query_pos == std::string::npos) {
            request.path = request.target;
        } else {
            request.path = request.target.substr(0, query_pos);
            request.query = request.target.substr(query_pos + 1);
        }

        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                break;
            }
            const std::size_t colon = line.find(':');
            if (colon == std::string::npos) {
                continue;
            }
            request.headers[trim(line.substr(0, colon))] = trim(line.substr(colon + 1));
        }
        return request;
    }

    static std::size_t content_length(const LocalHttpRequest& request) {
        for (const auto& header : request.headers) {
            if (lower_copy(header.first) == "content-length") {
                return static_cast<std::size_t>(std::stoul(header.second));
            }
        }
        return 0;
    }

    static LocalHttpRequest read_request(asio::ip::tcp::socket& socket) {
        asio::streambuf buffer;
        asio::read_until(socket, buffer, "\r\n\r\n");

        std::istream stream(&buffer);
        std::string head;
        std::string line;
        while (std::getline(stream, line)) {
            head += line;
            head += '\n';
            if (line == "\r") {
                break;
            }
        }

        LocalHttpRequest request = parse_request_head(head);
        std::ostringstream body_stream;
        body_stream << stream.rdbuf();
        request.body = body_stream.str();

        const std::size_t expected_body = content_length(request);
        while (request.body.size() < expected_body) {
            char chunk[4096];
            asio::error_code ec;
            const std::size_t received = socket.read_some(asio::buffer(chunk), ec);
            if (ec) {
                throw std::runtime_error("failed to read request body: " + ec.message());
            }
            request.body.append(chunk, chunk + received);
        }
        if (request.body.size() > expected_body) {
            request.body.resize(expected_body);
        }
        return request;
    }
};

} // namespace kurlyk_tests

#endif // KURLYK_TESTS_LOCAL_HTTP_SERVER_HPP_INCLUDED
