#pragma once
#ifndef _KURLYK_HTTP_CLIENT_HPP_INCLUDED
#define _KURLYK_HTTP_CLIENT_HPP_INCLUDED

/// \file HttpClient.hpp
/// \brief Contains the definition of the HttpClient class, which provides an interface for making HTTP requests to a specific host.

#include "../NetworkWorker.hpp"

namespace kurlyk {

	/// \class HttpClient
    /// \brief A client class for making HTTP requests to a specific host.
	/// This class provides methods to configure the client, including rate limiting, proxy settings, retry logic, and more.
	class HttpClient {
	public:

        /// \brief Default constructor for HttpClient.
		HttpClient() {
            ensure_initialized();
		}

		/// \brief Constructs an HttpClient with the specified host.
        /// \param host The base host URL for the HTTP client.
		HttpClient(const std::string& host) :
                m_host(host) {
            ensure_initialized();
		}

		HttpClient(const HttpClient&) = delete;
        void operator=(const HttpClient&) = delete;

		/// \brief Destructor for HttpClient.
        virtual ~HttpClient() {
            auto& instance = HttpRequestManager::get_instance();
            if (is_general_limit_owned) {
                instance.remove_limit(m_request.general_rate_limit_id);
            }
            if (is_specific_limit_owned) {
                instance.remove_limit(m_request.specific_rate_limit_id);
            }
        }

        /// \brief Sets the host URL for the HTTP client.
        /// \param host The base host URL for HTTP requests.
        void set_host(const std::string& host) {
            m_host = host;
        }

        /// \brief Sets the default headers for HTTP requests.
        /// \param headers The headers to be included with each request.
        void set_headers(const kurlyk::Headers& headers) {
            m_request.headers = headers;
        }

        /// \brief Assigns an existing rate limit to the HTTP request.
        /// \param limit_id The unique identifier of the rate limit to assign.
        /// \param type Specifies the rate limit type (general or specific).
        void assign_rate_limit_id(
                long limit_id,
                RateLimitType type = RateLimitType::General) {
            auto& instance = HttpRequestManager::get_instance();
            switch (type) {
            case RateLimitType::General:
                if (is_general_limit_owned) {
                    instance.remove_limit(m_request.general_rate_limit_id);
                }
                m_request.general_rate_limit_id = limit_id;
                is_general_limit_owned = false;
                break;
            case RateLimitType::Specific:
                if (is_specific_limit_owned) {
                    instance.remove_limit(m_request.specific_rate_limit_id);
                }
                m_request.specific_rate_limit_id = limit_id;
                is_specific_limit_owned = false;
                break;
            }
		}

		/// \brief Sets the rate limit ID for the HTTP request (alias for `assign_rate_limit_id`).
        /// \param limit_id The unique identifier of the rate limit to assign.
        /// \param type Specifies the rate limit type (general or specific).
        /// \note This method is an alias for `assign_rate_limit_id`.
        void set_rate_limit_id(
                long limit_id,
                RateLimitType type = RateLimitType::General) {
            assign_rate_limit_id(limit_id, type);
        }

        /// \brief Sets the rate limit for HTTP requests.
        /// \param requests_per_period The maximum number of requests allowed within the specified period.
        /// \param period_ms The duration of the period in milliseconds.
        /// \param type The type of rate limit (either general or specific).
		void set_rate_limit(
                long requests_per_period,
                long period_ms,
                RateLimitType type = RateLimitType::General) {
            auto& instance = HttpRequestManager::get_instance();
            switch (type) {
            case RateLimitType::General:
                if (is_general_limit_owned) {
                    instance.remove_limit(m_request.general_rate_limit_id);
                }
                m_request.general_rate_limit_id = instance.create_rate_limit(requests_per_period, period_ms);
                is_general_limit_owned = true;
                break;
            case RateLimitType::Specific:
                if (is_specific_limit_owned) {
                    instance.remove_limit(m_request.specific_rate_limit_id);
                }
                m_request.specific_rate_limit_id = instance.create_rate_limit(requests_per_period, period_ms);
                is_specific_limit_owned = true;
                break;
            }
		}

        /// \brief Sets the rate limit based on requests per minute (RPM).
        /// \param requests_per_minute Maximum number of requests allowed per minute.
        /// \param type The type of rate limit (either general or specific).
        void set_rate_limit_rpm(
                long requests_per_minute,
                RateLimitType type = RateLimitType::General) {
            long period_ms = 60000; // 1 minute in milliseconds
            return set_rate_limit(requests_per_minute, period_ms, type);
        }

        /// \brief Sets the rate limit based on requests per second (RPS).
        /// \param requests_per_second Maximum number of requests allowed per second.
        /// \param type The type of rate limit (either general or specific).
        void set_rate_limit_rps(
                long requests_per_second,
                RateLimitType type = RateLimitType::General) {
            long period_ms = 1000; // 1 second in milliseconds
            return set_rate_limit(requests_per_second, period_ms, type);
        }

		/// \brief Sets the Accept-Encoding header.
        /// \param identity Enable identity encoding.
        /// \param deflate Enable deflate encoding.
        /// \param gzip Enable gzip encoding.
        /// \param brotli Enable brotli encoding.
        void set_accept_encoding(
                bool identity = false,
                bool deflate = false,
                bool gzip = false,
                bool brotli = false) {
            m_request.set_accept_encoding(identity, deflate, gzip, brotli);
        }

        /// \brief Sets a custom Accept-Encoding header value.
        /// \param value The custom value for the Accept-Encoding header.
        void set_accept_encoding(const std::string& value) {
            m_request.accept_encoding = value;
        }

        /// \brief Sets the Accept-Language header value.
        /// \param value The language value to be sent with the Accept-Language header.
        void set_accept_language(const std::string& value) {
            m_request.headers.emplace("Accept-Language", value);
        }

        /// \brief Sets the Content-Type header value.
        /// \param value The MIME type for the Content-Type header.
        void set_content_type(const std::string& value) {
            m_request.headers.emplace("Content-Type", value);
        }

        /// \brief Sets the Origin header value.
        /// \param value The origin to be sent with the Origin header.
        void set_origin(const std::string& value) {
            m_request.headers.emplace("Origin", value);
        }

        /// \brief Sets the Referer header value.
        /// \param value The referer URL to be sent with the Referer header.
        void set_referer(const std::string& value) {
            m_request.headers.emplace("Referer", value);
        }

        /// \brief Sets the Do Not Track (DNT) header value.
        /// \param value If true, sets the DNT header to "1".
        void set_dnt(const bool value) {
            if (value) m_request.headers.emplace("dnt", "1");
        }

        /// \brief Configures whether to follow redirects automatically.
        /// \param value If true, enables following HTTP redirects.
        void set_follow_location(const bool value) {
            m_request.follow_location = value;
        }

        /// \brief Configures whether to automatically set the Referer header on redirects.
        /// \param value If true, enables automatically setting the Referer header during redirects.
        void set_auto_referer(const bool value) {
            m_request.auto_referer = value;
        }

        /// \brief Configures whether to use a tunneling proxy for HTTP requests.
        /// \param value If true, enables tunneling through the proxy server. Tunneling proxies are typically used for HTTPS requests to securely forward traffic.
        void set_proxy_tunnel(const bool value) {
            m_request.proxy_tunnel = value;
        }

        /// \brief Sets the proxy server address.
        /// \param ip Proxy server IP address.
        /// \param port Proxy server port.
        void set_proxy(const std::string& ip, int port) {
            m_request.set_proxy(ip, port);
        }

        /// \brief Sets the proxy server address with authentication details.
        /// \param ip Proxy server IP address.
        /// \param port Proxy server port.
        /// \param username Proxy username.
        /// \param password Proxy password.
        /// \param type The type of proxy, defaulting to HTTP.
        void set_proxy(
                const std::string& ip,
                const int port,
                const std::string& username,
                const std::string& password,
                ProxyType type = ProxyType::HTTP) {
            m_request.set_proxy(ip, port, username, password, type);
        }

        /// \brief Sets proxy authentication credentials.
        /// \param username Proxy username.
        /// \param password Proxy password.
        void set_proxy_auth(
                const std::string& username,
                const std::string& password) {
            m_request.set_proxy_auth(username, password);
        }

        /// \brief Sets retry attempts and delay between retries for HTTP requests.
        /// \param retry_attempts Number of retry attempts.
        /// \param retry_delay_ms Delay in milliseconds between retry attempts.
        void set_retry_attempts(long retry_attempts, long retry_delay_ms) {
            m_request.set_retry_attempts(retry_attempts, retry_delay_ms);
        }

        /// \brief Sets the User-Agent header.
        /// \param user_agent User-Agent string.
        void set_user_agent(const std::string& user_agent) {
            m_request.set_user_agent(user_agent);
        }

        /// \brief Sets the cookie string for HTTP requests.
        /// \param cookie Cookie data as a single string.
        void set_cookie(const std::string& cookie) {
            m_request.set_cookie(cookie);
        }

        /// \brief Sets the client certificate file path.
        /// \param cert_file Path to the client certificate file.
        void set_cert_file(const std::string& cert_file) {
            m_request.set_cert_file(cert_file);
        }

        /// \brief Sets the path to the CA certificate file.
        /// \param ca_file Path to the CA certificate file.
        void set_ca_file(const std::string& ca_file) {
            m_request.set_ca_file(ca_file);
        }

        /// \brief Sets the timeout duration for HTTP requests.
        /// \param timeout Timeout duration in seconds.
        void set_timeout(long timeout) {
            m_request.set_timeout(timeout);
        }

        /// \brief Sets the connection timeout duration.
        /// \param connect_timeout Connection timeout in seconds.
        void set_connect_timeout(long connect_timeout) {
            m_request.set_connect_timeout(connect_timeout);
        }

        /// \brief Enables or disables verbose output.
        /// \param verbose Enable (true) or disable (false) verbose output.
        void set_verbose(bool verbose) {
            m_request.verbose = verbose;
        }

        /// \brief Enables or disables debug headers.
        /// \param debug_header Enable (true) or disable (false) debug headers.
        void set_debug_header(bool debug_header) {
            m_request.debug_header = debug_header;
        }

        /// \brief Sets the maximum number of redirects for the client.
        /// \param max_redirects The maximum number of redirects allowed.
        void set_max_redirects(long max_redirects) {
            m_request.max_redirects = max_redirects;
        }

        /// \brief Sends an HTTP request with the specified method, path, and parameters.
        /// \param method The HTTP method (e.g., "GET", "POST").
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param content The request body content.
        /// \param callback The callback function to be called when the request is completed.
        /// \return true if the request was successfully added to the RequestManager; false otherwise.
		bool request(
                const std::string &method,
                const std::string& path,
                const QueryParams &query,
                const Headers &headers,
                const std::string &content,
                HttpResponseCallback callback) {
#           if __cplusplus >= 201402L
            std::unique_ptr<HttpRequest> request_ptr = std::make_unique<HttpRequest>(m_request);
#           else
            std::unique_ptr<HttpRequest> request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest(m_request));
#           endif
            request_ptr->method = method;
            request_ptr->set_url(m_host, path, query);
            request_ptr->headers.insert(headers.begin(), headers.end());
            request_ptr->content = content;
            return request(std::move(request_ptr), std::move(callback));
		}

        /// \brief Sends an HTTP request with the specified method, path, parameters, and specific rate limit ID.
        /// \param method The HTTP method (e.g., "GET", "POST").
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param content The request body content.
        /// \param specific_rate_limit_id The specific rate limit ID to be applied to this request.
        /// \param callback The callback function to be called when the request is completed.
        /// \return true if the request was successfully added to the RequestManager; false otherwise.
        bool request(
                const std::string &method,
                const std::string& path,
                const QueryParams &query,
                const Headers &headers,
                const std::string &content,
                long specific_rate_limit_id,
                HttpResponseCallback callback) {
#           if __cplusplus >= 201402L
            std::unique_ptr<HttpRequest> request_ptr = std::make_unique<HttpRequest>(m_request);
#           else
            std::unique_ptr<HttpRequest> request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest(m_request));
#           endif
            request_ptr->method = method;
            request_ptr->set_url(m_host, path, query);
            request_ptr->headers.insert(headers.begin(), headers.end());
            request_ptr->content = content;

            // Set the specific rate limit ID for this request
            if (is_specific_limit_owned) {
                HttpRequestManager::get_instance().remove_limit(request_ptr->specific_rate_limit_id);
            }
            request_ptr->specific_rate_limit_id = specific_rate_limit_id;
            is_specific_limit_owned = false;

            return request(std::move(request_ptr), std::move(callback));
        }

		/// \brief Sends a GET request.
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param callback The callback function to be called when the request is completed.
        /// \return true if the request was successfully added to the RequestManager; false otherwise.
		bool get(
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                HttpResponseCallback callback) {
            return request("GET", path, query, headers, std::string(), std::move(callback));
        }

        /// \brief Sends a POST request.
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param content The request body content.
        /// \param callback The callback function to be called when the request is completed.
        /// \return true if the request was successfully added to the RequestManager; false otherwise.
        bool post(
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                const std::string& content,
                HttpResponseCallback callback) {
            return request("POST", path, query, headers, content, std::move(callback));
        }

        /// \brief Sends a GET request with a specific rate limit ID.
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param specific_rate_limit_id The specific rate limit ID to be applied to this request.
        /// \param callback The callback function to be called when the request is completed.
        /// \return true if the request was successfully added to the RequestManager; false otherwise.
        bool get(
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                long specific_rate_limit_id,
                HttpResponseCallback callback) {
            return request("GET", path, query, headers, std::string(), specific_rate_limit_id, std::move(callback));
        }

        /// \brief Sends a POST request with a specific rate limit ID.
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param content The request body content.
        /// \param specific_rate_limit_id The specific rate limit ID to be applied to this request.
        /// \param callback The callback function to be called when the request is completed.
        /// \return true if the request was successfully added to the RequestManager; false otherwise.
        bool post(
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                const std::string& content,
                long specific_rate_limit_id,
                HttpResponseCallback callback) {
            return request("POST", path, query, headers, content, specific_rate_limit_id, std::move(callback));
        }

        /// \brief Sends an HTTP request with a specified method, path, and parameters, and returns a future with the response.
        /// \param method The HTTP method (e.g., "GET", "POST").
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param content The request body content.
        /// \return A future containing the HttpResponsePtr object.
        std::future<HttpResponsePtr> request(
                const std::string& method,
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                const std::string& content) {
#           if __cplusplus >= 201402L
            auto request_ptr = std::make_unique<HttpRequest>(m_request);
#           else
            auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest(m_request));
#           endif
            request_ptr->method = method;
            request_ptr->set_url(m_host, path, query);
            request_ptr->headers.insert(headers.begin(), headers.end());
            request_ptr->content = content;

            auto promise = std::make_shared<std::promise<HttpResponsePtr>>();
            auto future = promise->get_future();

            HttpResponseCallback callback = [promise](HttpResponsePtr response) {
                if (!response->ready) return;
                promise->set_value(std::move(response));
            };

            if (!request(std::move(request_ptr), std::move(callback))) {
                promise->set_exception(std::make_exception_ptr(
                    std::runtime_error("Failed to add request to RequestManager")));
            }

            return future;
        }

        /// \brief Sends an HTTP request with a specified method, path, specific rate limit ID and parameters, and returns a future with the response.
        /// \param method The HTTP method (e.g., "GET", "POST").
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param content The request body content.
        /// \param specific_rate_limit_id The specific rate limit ID to be applied to this request.
        /// \return A future containing the HttpResponsePtr object.
        std::future<HttpResponsePtr> request(
                const std::string& method,
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                const std::string& content,
                long specific_rate_limit_id) {
#           if __cplusplus >= 201402L
            auto request_ptr = std::make_unique<HttpRequest>(m_request);
#           else
            auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest(m_request));
#           endif
            request_ptr->method = method;
            request_ptr->set_url(m_host, path, query);
            request_ptr->headers.insert(headers.begin(), headers.end());
            request_ptr->content = content;

            // Set the specific rate limit ID for this request
            if (is_specific_limit_owned) {
                HttpRequestManager::get_instance().remove_limit(request_ptr->specific_rate_limit_id);
            }
            request_ptr->specific_rate_limit_id = specific_rate_limit_id;
            is_specific_limit_owned = false;

            auto promise = std::make_shared<std::promise<HttpResponsePtr>>();
            auto future = promise->get_future();

            HttpResponseCallback callback = [promise](HttpResponsePtr response) {
                if (!response->ready) return;
                promise->set_value(std::move(response));
            };

            if (!request(std::move(request_ptr), std::move(callback))) {
                promise->set_exception(std::make_exception_ptr(
                    std::runtime_error("Failed to add request to RequestManager")));
            }

            return future;
        }

        /// \brief Sends a GET request asynchronously and returns a future with the response.
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \return A future containing the HttpResponsePtr object.
        std::future<HttpResponsePtr> get(
                const std::string& path,
                const QueryParams& query,
                const Headers& headers) {
            return request("GET", path, query, headers, std::string());
        }

        /// \brief Sends a POST request asynchronously and returns a future with the response.
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param content The request body content.
        /// \return A future containing the HttpResponsePtr object.
        std::future<HttpResponsePtr> post(
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                const std::string& content) {
            return request("POST", path, query, headers, content);
        }

        /// \brief Sends an asynchronous GET request with a specific rate limit ID and returns a future with the response.
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param specific_rate_limit_id The specific rate limit ID to be applied to this request.
        /// \return A future containing the HttpResponsePtr object.
        std::future<HttpResponsePtr> get(
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                long specific_rate_limit_id) {
            return request("GET", path, query, headers, std::string(), specific_rate_limit_id);
        }

        /// \brief Sends an asynchronous POST request with a specific rate limit ID and returns a future with the response.
        /// \param path The URL path for the request.
        /// \param query The query arguments.
        /// \param headers The HTTP headers.
        /// \param content The request body content.
        /// \param specific_rate_limit_id The specific rate limit ID to be applied to this request.
        /// \return A future containing the HttpResponsePtr object.
        std::future<HttpResponsePtr> post(
                const std::string& path,
                const QueryParams& query,
                const Headers& headers,
                const std::string& content,
                long specific_rate_limit_id) {
            return request("POST", path, query, headers, content, specific_rate_limit_id);
        }

	private:
        HttpRequest m_request;  ///< The request object used for configuring and sending requests.
        std::string m_host;     ///< The base host URL for the HTTP client.
        bool is_general_limit_owned = false;
        bool is_specific_limit_owned = false;

        /// \brief Adds the request to the request manager and notifies the worker to process it.
        /// \param request_ptr The HTTP request to be sent.
        /// \param callback The callback function to be called when the request is completed.
        /// \return true if the request was successfully added to the RequestManager; false otherwise.
        bool request(
                std::unique_ptr<HttpRequest> request_ptr,
                HttpResponseCallback callback) {
            const bool status = HttpRequestManager::get_instance().add_request(std::move(request_ptr), std::move(callback));
            NetworkWorker::get_instance().notify();
            return status;
		}

        /// \brief Ensures that the network worker and request manager are initialized.
		static void ensure_initialized() {
            static bool is_initialized = false;
            if (!is_initialized) {
                is_initialized = true;
                HttpRequestManager::get_instance();
                NetworkWorker::get_instance().start(true);
            }
        }

	}; // HttpClient

}; // namespace kurlyk

#endif // _KURLYK_HTTP_CLIENT_HPP_INCLUDED
