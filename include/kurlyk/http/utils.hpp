#pragma once
#ifndef _KURLYK_HTTP_UTILS_HPP_INCLUDED
#define _KURLYK_HTTP_UTILS_HPP_INCLUDED

/// \file utils.hpp
/// \brief Contains utility functions for handling HTTP requests, rate limiting, and request cancellation.

namespace kurlyk {

    /// \brief Creates a rate limit with specified parameters.
    /// \param requests_per_period Maximum number of requests allowed within the specified period.
    /// \param period_ms Time period in milliseconds for the rate limit.
    /// \return A unique identifier for the created rate limit.
    long create_rate_limit(long requests_per_period, long period_ms) {
        return HttpRequestManager::get_instance().create_rate_limit(requests_per_period, period_ms);
    }

    /// \brief Creates a rate limit based on Requests Per Minute (RPM).
    /// \param requests_per_minute Maximum number of requests allowed per minute.
    /// \return A unique identifier for the created rate limit.
    long create_rate_limit_rpm(long requests_per_minute) {
        long period_ms = 60000; // 1 minute in milliseconds
        return HttpRequestManager::get_instance().create_rate_limit(requests_per_minute, period_ms);
    }

    /// \brief Creates a rate limit based on Requests Per Second (RPS).
    /// \param requests_per_second Maximum number of requests allowed per second.
    /// \return A unique identifier for the created rate limit.
    long create_rate_limit_rps(long requests_per_second) {
        long period_ms = 1000; // 1 second in milliseconds
        return HttpRequestManager::get_instance().create_rate_limit(requests_per_second, period_ms);
    }

    /// \brief Removes an existing rate limit with the specified identifier.
    /// \param limit_id The unique identifier of the rate limit to be removed.
    /// \return True if the rate limit was successfully removed, or false if the rate limit ID was not found.
    bool remove_limit(long limit_id) {
        return HttpRequestManager::get_instance().remove_limit(limit_id);
    }

    /// \brief Generates a new unique request ID.
    /// \return A new unique request ID.
    uint64_t generate_request_id() {
        return HttpRequestManager::get_instance().generate_request_id();
    }

    /// \brief Cancels a request by its unique identifier.
    /// \param request_id The unique identifier of the request to cancel.
    /// \param callback An optional callback function to execute after cancellation.
    void cancel_request_by_id(uint64_t request_id, std::function<void()> callback) {
        HttpRequestManager::get_instance().cancel_request_by_id(request_id, std::move(callback));
        ::kurlyk::core::NetworkWorker::get_instance().notify();
    }

    /// \brief Cancels a request by its unique identifier and returns a future.
    /// \param request_id The unique identifier of the request to cancel.
    /// \return A `std::future<void>` that becomes ready when the cancellation process is complete.
    std::future<void> cancel_request_by_id(uint64_t request_id) {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();
        HttpRequestManager::get_instance().cancel_request_by_id(request_id, [promise](){
            try {
                promise->set_value();
            } catch (const std::future_error& e) {
                if (e.code() == std::make_error_condition(std::future_errc::promise_already_satisfied)) {
                    KURLYK_HANDLE_ERROR(e, "Promise already satisfied in HttpClient::request callback");
                } else {
                    KURLYK_HANDLE_ERROR(e, "Future error in HttpClient::request callback");
                }
            } catch (const std::exception& e) {
                KURLYK_HANDLE_ERROR(e, "Unhandled exception in HttpClient::request callback");
            } catch (...) {
                // Unknown fatal error in request callback
            }
        });
        ::kurlyk::core::NetworkWorker::get_instance().notify();
        return future;
    }

    /// \brief Sends an HTTP request with callback.
    /// \param request_ptr The HTTP request object with the request details.
    /// \param callback The callback function to be called upon request completion.
    /// \return True if the request was successfully added to the manager, false otherwise.
    bool http_request(
            std::unique_ptr<HttpRequest> request_ptr,
            HttpResponseCallback callback) {
        const bool status = HttpRequestManager::get_instance().add_request(std::move(request_ptr), std::move(callback));
        ::kurlyk::core::NetworkWorker::get_instance().notify();
        return status;
    }

    /// \brief Sends an HTTP request asynchronously and returns a future with the response.
    /// \param request_ptr The HTTP request object with the request details.
    /// \return A future containing the HttpResponsePtr with the response details.
    std::future<HttpResponsePtr> http_request(
            std::unique_ptr<HttpRequest> request_ptr) {

        auto promise = std::make_shared<std::promise<HttpResponsePtr>>();
        auto future = promise->get_future();

        HttpResponseCallback callback = [promise](HttpResponsePtr response) {
            if (!response || !response->ready) return;
            try {
                promise->set_value(std::move(response));
            } catch (const std::future_error& e) {
                if (e.code() == std::make_error_condition(std::future_errc::promise_already_satisfied)) {
                    KURLYK_HANDLE_ERROR(e, "Promise already satisfied in HttpClient::request callback");
                } else {
                    KURLYK_HANDLE_ERROR(e, "Future error in HttpClient::request callback");
                }
            } catch (const std::exception& e) {
                KURLYK_HANDLE_ERROR(e, "Unhandled exception in HttpClient::request callback");
            } catch (...) {
                // Unknown fatal error in request callback
            }
        };

        const bool status = HttpRequestManager::get_instance().add_request(std::move(request_ptr), std::move(callback));
        if (!status) {
            try {
                promise->set_exception(std::make_exception_ptr(
                    std::runtime_error(
                        "Failed to add request to RequestManager")));
            } catch (...) {}
        }
        ::kurlyk::core::NetworkWorker::get_instance().notify();

        return future;
    }

    /// \brief Sends an HTTP request with detailed parameters and a callback.
    /// \param method HTTP method (e.g., "GET", "POST").
    /// \param url The full request URL.
    /// \param query Query parameters for the request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for POST requests.
    /// \param callback Callback function to be executed upon request completion.
    /// \return The unique identifier of the HTTP request if successfully added, or 0 on failure.
    uint64_t http_request(
            const std::string &method,
            const std::string &url,
            const QueryParams &query,
            const Headers &headers,
            const std::string &content,
            HttpResponseCallback callback) {
#       if __cplusplus >= 201402L
        auto request_ptr = std::make_unique<HttpRequest>();
#       else
        auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest());
#       endif

        const uint64_t request_id = HttpRequestManager::get_instance().generate_request_id();
        request_ptr->request_id = request_id;
        request_ptr->set_url(url, query);
        request_ptr->method  = method;
        request_ptr->headers = headers;
        request_ptr->content = content;
        if (http_request(std::move(request_ptr), std::move(callback))) return request_id;
        return 0;
    }

    /// \brief Sends an HTTP request asynchronously with detailed parameters and returns a future.
    /// \param method HTTP method (e.g., "GET", "POST").
    /// \param url The full request URL.
    /// \param query Query parameters for the request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for POST requests.
    /// \return A pair containing the unique request ID and a future with the response details.
    std::pair<uint64_t, std::future<HttpResponsePtr>> http_request(
            const std::string &method,
            const std::string &url,
            const QueryParams &query,
            const Headers &headers,
            const std::string &content) {

#       if __cplusplus >= 201402L
        auto request_ptr = std::make_unique<HttpRequest>();
#       else
        auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest());
#       endif

        const uint64_t request_id = HttpRequestManager::get_instance().generate_request_id();
        request_ptr->request_id = request_id;
        request_ptr->set_url(url, query);
        request_ptr->method = method;
        request_ptr->headers = headers;
        request_ptr->content = content;

        auto promise = std::make_shared<std::promise<HttpResponsePtr>>();
        auto future = promise->get_future();

        HttpResponseCallback callback = [promise](HttpResponsePtr response) {
            if (!response || !response->ready) return;
            try {
                promise->set_value(std::move(response));
            } catch (const std::future_error& e) {
                if (e.code() == std::make_error_condition(std::future_errc::promise_already_satisfied)) {
                    KURLYK_HANDLE_ERROR(e, "Promise already satisfied in HttpClient::request callback");
                } else {
                    KURLYK_HANDLE_ERROR(e, "Future error in HttpClient::request callback");
                }
            } catch (const std::exception& e) {
                KURLYK_HANDLE_ERROR(e, "Unhandled exception in HttpClient::request callback");
            } catch (...) {
                // Unknown fatal error in request callback
            }
        };

        if (!http_request(std::move(request_ptr), std::move(callback))) {
            try {
                promise->set_exception(std::make_exception_ptr(
                    std::runtime_error(
                        "Failed to add request to HttpRequestManager")));
            } catch (...) {}
        }

        return {request_id, std::move(future)};
    }

    /// \brief Sends an HTTP request with a specified host, path, and callback.
    /// \param method HTTP method (e.g., "GET", "POST").
    /// \param host Host name or IP address.
    /// \param path URL path for the request.
    /// \param query Query parameters for the request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for POST requests.
    /// \param callback Callback function to be executed upon request completion.
    /// \return The unique identifier of the HTTP request if successfully added, or 0 on failure.
    uint64_t http_request(
            const std::string &method,
            const std::string &host,
            const std::string &path,
            const QueryParams &query,
            const Headers &headers,
            const std::string &content,
            HttpResponseCallback callback) {

#       if __cplusplus >= 201402L
        auto request_ptr = std::make_unique<HttpRequest>();
#       else
        auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest());
#       endif

        const uint64_t request_id = HttpRequestManager::get_instance().generate_request_id();
        request_ptr->request_id = request_id;
        request_ptr->set_url(host, path, query);
        request_ptr->method  = method;
        request_ptr->headers = headers;
        request_ptr->content = content;
        if (http_request(std::move(request_ptr), std::move(callback))) return request_id;
        return 0;
    }

    /// \brief Sends an asynchronous HTTP GET request with a callback.
    /// \param url The full request URL.
    /// \param query Query parameters for the GET request.
    /// \param headers HTTP headers to include.
    /// \param callback Callback function to be executed upon request completion.
    /// \return The unique identifier of the HTTP request if successfully added, or 0 on failure.
    uint64_t http_get(
            const std::string &url,
            const QueryParams& query,
            const Headers& headers,
            HttpResponseCallback callback) {
        return http_request("GET", url, query, headers, std::string(), std::move(callback));
    }

    /// \brief Sends an asynchronous HTTP GET request and returns a future with the response.
    /// \param url The full request URL.
    /// \param query Query parameters for the GET request.
    /// \param headers HTTP headers to include.
    /// \return A pair containing the unique request ID and a future with the response details.
    std::pair<uint64_t, std::future<HttpResponsePtr>> http_get(
            const std::string& url,
            const QueryParams& query,
            const Headers& headers) {

#       if __cplusplus >= 201402L
        auto request_ptr = std::make_unique<HttpRequest>();
#       else
        auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest());
#       endif

        const uint64_t request_id = HttpRequestManager::get_instance().generate_request_id();
        request_ptr->request_id = request_id;
        request_ptr->set_url(url, query);
        request_ptr->method = "GET";
        request_ptr->headers = headers;

        auto promise = std::make_shared<std::promise<HttpResponsePtr>>();
        auto future = promise->get_future();

        HttpResponseCallback callback = [promise](HttpResponsePtr response) {
            if (!response || !response->ready) return;
            try {
                promise->set_value(std::move(response));
            } catch (const std::future_error& e) {
                if (e.code() == std::make_error_condition(std::future_errc::promise_already_satisfied)) {
                    KURLYK_HANDLE_ERROR(e, "Promise already satisfied in HttpClient::request callback");
                } else {
                    KURLYK_HANDLE_ERROR(e, "Future error in HttpClient::request callback");
                }
            } catch (const std::exception& e) {
                KURLYK_HANDLE_ERROR(e, "Unhandled exception in HttpClient::request callback");
            } catch (...) {
                // Unknown fatal error in request callback
            }
        };

        if (!http_request(std::move(request_ptr), std::move(callback))) {
            try {
                promise->set_exception(std::make_exception_ptr(
                    std::runtime_error(
                        "Failed to add request to HttpRequestManager")));
            } catch (...) {}
        }

        return {request_id, std::move(future)};
    }

    /// \brief Sends an asynchronous HTTP POST request with a callback.
    /// \param url The full request URL.
    /// \param query Query parameters for the POST request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for the POST request.
    /// \param callback Callback function to be executed upon request completion.
    /// \return The unique identifier of the HTTP request if successfully added, or 0 on failure.
    uint64_t http_post(
            const std::string &url,
            const QueryParams& query,
            const Headers& headers,
            const std::string& content,
            HttpResponseCallback callback) {
        return http_request("POST", url, query, headers, content, std::move(callback));
    }

    /// \brief Sends an asynchronous HTTP POST request and returns a future with the response.
    /// \param url The full request URL.
    /// \param query Query parameters for the POST request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for the POST request.
    /// \return A pair containing the unique request ID and a future with the response details.
    std::pair<uint64_t, std::future<HttpResponsePtr>> http_post(
            const std::string& url,
            const QueryParams& query,
            const Headers& headers,
            const std::string& content) {

#       if __cplusplus >= 201402L
        auto request_ptr = std::make_unique<HttpRequest>();
#       else
        auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest());
#       endif

        const uint64_t request_id = HttpRequestManager::get_instance().generate_request_id();
        request_ptr->request_id = request_id;
        request_ptr->set_url(url, query);
        request_ptr->method = "POST";
        request_ptr->headers = headers;
        request_ptr->content = content;

        auto promise = std::make_shared<std::promise<HttpResponsePtr>>();
        auto future = promise->get_future();

        HttpResponseCallback callback = [promise](HttpResponsePtr response) {
            if (!response || !response->ready) return;
            try {
                promise->set_value(std::move(response));
            } catch (const std::future_error& e) {
                if (e.code() == std::make_error_condition(std::future_errc::promise_already_satisfied)) {
                    KURLYK_HANDLE_ERROR(e, "Promise already satisfied in HttpClient::request callback");
                } else {
                    KURLYK_HANDLE_ERROR(e, "Future error in HttpClient::request callback");
                }
            } catch (const std::exception& e) {
                KURLYK_HANDLE_ERROR(e, "Unhandled exception in HttpClient::request callback");
            } catch (...) {
                // Unknown fatal error in request callback
            }
        };

        if (!http_request(std::move(request_ptr), std::move(callback))) {
            try {
                promise->set_exception(std::make_exception_ptr(
                    std::runtime_error(
                        "Failed to add request to HttpRequestManager")));
            } catch (...) {}
        }

        return {request_id, std::move(future)};
    }

} // namespace kurlyk

#endif // _KURLYK_HTTP_UTILS_HPP_INCLUDED
