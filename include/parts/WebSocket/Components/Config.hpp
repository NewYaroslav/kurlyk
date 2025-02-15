#pragma once
#ifndef _KURLYK_WEB_SOCKET_CONFIG_HPP_INCLUDED
#define _KURLYK_WEB_SOCKET_CONFIG_HPP_INCLUDED

/// \file Config.hpp
/// \brief Defines the configuration options for WebSocket connections.

#include "../../Utils.hpp"

namespace kurlyk {

    /// \class WebSocketConfig
    /// \brief Configuration parameters for establishing and managing WebSocket connections.
    class WebSocketConfig {
    public:
        Headers headers;                ///< HTTP headers included in the WebSocket connection request.
        std::string url;                ///< URL of the WebSocket server.
        std::string user_agent;         ///< User-Agent header.
        std::string accept_encoding;    ///< Accept-Encoding header.
        std::string cookie;             ///< Cookie data as a string.
        std::vector<std::string> protocols; ///< List of subprotocols for the Sec-WebSocket-Protocol header.
        std::string cert_file;          ///< Path to the client certificate file.
        std::string key_file;           ///< Path to the private key file corresponding to the client certificate.
        std::string ca_file;            ///< Path to the Root CA certificate file.
        std::string proxy_server;       ///< Proxy address in <ip:port> format.
        std::string proxy_auth;         ///< Proxy authentication in <username:password> format.
        ProxyType proxy_type    = ProxyType::HTTP; ///< Proxy type (e.g., HTTP, SOCKS5).
        long request_timeout    = 20;   ///< Timeout for WebSocket requests in seconds (0 means no timeout).
        long idle_timeout       = 0;    ///< Maximum idle time for the WebSocket connection in seconds (0 means no timeout).
        long reconnect_delay    = 5;    ///< Delay in seconds between reconnection attempts.
        long reconnect_attempts = 0;    ///< Number of reconnection attempts (0 means infinite attempts).
        bool reconnect      = true;     ///< Enables automatic reconnection if true.
        bool verify_cert    = true;     ///< If true, verifies the server’s certificate and hostname according to RFC 2818.

        /// \struct RateLimitData
        /// \brief Defines rate limit parameters.
        struct RateLimitData {
            long requests_per_period; ///< Maximum number of requests allowed per period.
            long period_ms; ///< Time period in milliseconds for the request limit.

            RateLimitData(long requests_per_period = 0, long period_ms = 0)
                : requests_per_period(requests_per_period), period_ms(period_ms) {}
        };

        std::vector<RateLimitData> rate_limits; ///< List of rate limits applied to WebSocket messages.

        /// \brief Sets the WebSocket server URL with optional query parameters.
        /// \param host Hostname or IP address.
        /// \param path Path for the request.
        /// \param query Optional query parameters.
        void set_url(const std::string& host, const std::string& path, const std::string& query = "") {
            url = host;
            if (!path.empty() && path[0] != '/') {
                url += "/";
            }
            url += path;
            if (!query.empty()) {
                url += (query[0] == '?' ? "" : "?") + query;
            }
        }

        /// \brief Sets the WebSocket server URL with specified query parameters.
        /// \param url Full URL of the server.
        /// \param query Query parameters as a dictionary.
        void set_url(const std::string& url, const QueryParams& query) {
            this->url = url + (query.empty() ? "" : utils::to_query_string(query, "?"));
        }

        /// \brief Sets the Accept-Encoding header with specified encodings.
        /// \param identity Enables identity encoding.
        /// \param deflate Enables deflate encoding.
        /// \param gzip Enables gzip encoding.
        /// \param brotli Enables brotli encoding.
        void set_accept_encoding(bool identity = false, bool deflate = false, bool gzip = false, bool brotli = false) {
            std::string encodings;
            if (identity) encodings += "identity";
            if (deflate) encodings += (encodings.empty() ? "" : ",") + std::string("deflate");
            if (gzip) encodings += (encodings.empty() ? "" : ",") + std::string("gzip");
            if (brotli) encodings += (encodings.empty() ? "" : ",") + std::string("br");
            accept_encoding = std::move(encodings);
        }

        /// \brief Sets the proxy server address.
        /// \param ip Proxy server IP address.
        /// \param port Proxy server port.
        void set_proxy(const std::string& ip, int port) {
            proxy_server = ip + ":" + std::to_string(port);
        }

        /// \brief Sets the proxy server address.
        /// \param server Proxy address in <ip:port> format.
        void set_proxy_server(const std::string& server) {
            proxy_server = server;
        }

        /// \brief Sets the proxy authentication credentials.
        /// \param auth Proxy authentication in <username:password> format.
        void set_proxy_auth(const std::string& auth) {
            proxy_auth = auth;
        }

        /// \brief Sets the proxy type.
        /// \param type Type of proxy.
        void set_proxy_type(ProxyType type) {
            proxy_type = type;
        }

        /// \brief Sets the proxy server address with authentication.
        /// \param ip Proxy server IP address.
        /// \param port Proxy server port.
        /// \param username Proxy username.
        /// \param password Proxy password.
        /// \param type Type of proxy (default is HTTP).
        void set_proxy(const std::string& ip, int port, const std::string& username, const std::string& password, ProxyType type = ProxyType::HTTP) {
            set_proxy(ip, port);
            set_proxy_auth(username, password);
            proxy_type = type;
        }

        /// \brief Configures proxy authentication credentials.
        /// \param username Proxy username.
        /// \param password Proxy password.
        void set_proxy_auth(const std::string& username, const std::string& password) {
            proxy_auth = username + ":" + password;
        }

        /// \brief Configures reconnection behavior.
        /// \param reconnect Enables automatic reconnection.
        /// \param reconnect_attempts Number of reconnection attempts (0 means infinite attempts).
        /// \param reconnect_delay Delay in seconds between reconnection attempts.
        void set_reconnect(bool reconnect, long reconnect_attempts = 0, long reconnect_delay = 0) {
            this->reconnect = reconnect;
            this->reconnect_attempts = reconnect_attempts;
            this->reconnect_delay = reconnect_delay;
        }

        /// \brief Sets the User-Agent header.
        /// \param user_agent User-Agent string.
        void set_user_agent(const std::string& user_agent) {
            this->user_agent = user_agent;
        }

        /// \brief Sets the Accept-Language header.
        /// \param accept_language Accept-Language string.
        void set_accept_language(const std::string& accept_language) {
            this->headers.emplace("Accept-Language", accept_language);
        }

        /// \brief Sets the cookie data.
        /// \param cookie Cookie data string.
        void set_cookie(const std::string& cookie) {
            this->cookie = cookie;
        }

        /// \brief Configures the idle timeout for the WebSocket connection.
        /// \param idle_timeout Idle timeout in seconds (0 means no timeout).
        void set_idle_timeout(long idle_timeout) {
            this->idle_timeout = idle_timeout;
        }

        /// \brief Sets the timeout for WebSocket requests.
        /// \param request_timeout Request timeout in seconds (0 means no timeout).
        void set_request_timeout(long request_timeout) {
            this->request_timeout = request_timeout;
        }

        /// \brief Sets the path to the CA certificate file.
        /// \param ca_file Path to the CA certificate file.
        void set_ca_file(const std::string& ca_file) {
            this->ca_file = ca_file;
        }

        /// \brief Sets certificate verification and sets the CA certificate file.
        /// \param verify_cert If true, enables server certificate verification.
        /// \param ca_file Path to the CA certificate file.
        void set_ca_file(bool verify_cert, const std::string& ca_file) {
            this->verify_cert = verify_cert;
            this->ca_file = ca_file;
        }

        /// \brief Sets whether to verify the server’s certificate.
        /// \param verify_cert If true, enables server certificate verification.
        void set_verify_cert(bool verify_cert) {
            this->verify_cert = verify_cert;
        }

        /// \brief Adds a rate limit configuration to control the frequency of WebSocket messages.
        ///
        /// The first rate limit added will serve as the primary rate limit, applying to all WebSocket
        /// requests by default. Additional rate limits can be added for specific types of requests
        /// or contexts as needed.
        ///
        /// \param requests_per_period The maximum number of messages allowed within the specified period.
        /// \param period_ms The time period in milliseconds during which the request limit applies.
        /// \return The index of the added rate limit configuration.
        long add_rate_limit(long requests_per_period, long period_ms) {
            rate_limits.emplace_back(requests_per_period, period_ms);
            return rate_limits.size() - 1;
        }

        /// \brief Adds a rate limit based on Requests Per Minute (RPM).
        /// \param requests_per_minute Maximum number of requests allowed per minute.
        /// \return The index of the added rate limit configuration.
        long add_rate_limit_rpm(long requests_per_minute) {
            long period_ms = 60000; // 1 minute in milliseconds
            return add_rate_limit(requests_per_minute, period_ms);
        }

        /// \brief Adds a rate limit based on Requests Per Second (RPS).
        /// \param requests_per_second Maximum number of requests allowed per second.
        /// \return The index of the added rate limit configuration.
        long add_rate_limit_rps(long requests_per_second) {
            long period_ms = 1000; // 1 second in milliseconds
            return add_rate_limit(requests_per_second, period_ms);
        }
    };

} // namespace kurlyk

#endif // _KURLYK_WEB_SOCKET_CONFIG_HPP_INCLUDED
