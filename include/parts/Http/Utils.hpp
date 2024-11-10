#pragma once
#ifndef _KURLYK_HTTP_UTILS_HPP_INCLUDED
#define _KURLYK_HTTP_UTILS_HPP_INCLUDED

/// \file Utils.hpp
/// \brief Contains utility functions for handling HTTP requests and rate limiting.

#include "RequestManager.hpp"
#include <future>

namespace kurlyk {

    /// \brief Creates a rate limit with specified parameters.
    /// \param requests_per_period Maximum number of requests allowed within the specified period.
    /// \param period_ms Time period in milliseconds for the rate limit.
    /// \return A unique identifier for the created rate limit.
    long create_rate_limit(const long& requests_per_period, const long& period_ms) {
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

    /// \brief Sends an HTTP request with callback.
    /// \param request_ptr The HTTP request object with the request details.
    /// \param callback The callback function to be called upon request completion.
    /// \return True if the request was successfully added to the manager, false otherwise.
    bool http_request(
            std::unique_ptr<HttpRequest> request_ptr,
            HttpResponseCallback callback) {
        const bool status = HttpRequestManager::get_instance().add_request(std::move(request_ptr), std::move(callback));
        NetworkWorker::get_instance().notify();
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
            if (!response->ready) return;
            promise->set_value(std::move(response));
        };

        const bool status = HttpRequestManager::get_instance().add_request(std::move(request_ptr), std::move(callback));
        if (!status) {
            promise->set_exception(std::make_exception_ptr(
                std::runtime_error("Failed to add request to RequestManager")));
        }
        NetworkWorker::get_instance().notify();

        return future;
    }

    /// \brief Sends an HTTP request with detailed parameters and a callback.
    /// \param method HTTP method (e.g., "GET", "POST").
    /// \param url The full request URL.
    /// \param query Query parameters for the request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for POST requests.
    /// \param callback Callback function to be executed upon request completion.
    /// \return True if the request was successfully added to the manager, false otherwise.
    bool http_request(
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

        request_ptr->set_url(url, query);
        request_ptr->method  = method;
        request_ptr->headers = headers;
        request_ptr->content = content;
        return http_request(std::move(request_ptr), std::move(callback));
    }

    /// \brief Sends an HTTP request asynchronously with detailed parameters and returns a future.
    /// \param method HTTP method (e.g., "GET", "POST").
    /// \param url The full request URL.
    /// \param query Query parameters for the request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for POST requests.
    /// \return A future that will contain the HttpResponsePtr with the response details.
    std::future<HttpResponsePtr> http_request(
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

        request_ptr->set_url(url, query);
        request_ptr->method = method;
        request_ptr->headers = headers;
        request_ptr->content = content;

        auto promise = std::make_shared<std::promise<HttpResponsePtr>>();
        auto future = promise->get_future();

        HttpResponseCallback callback = [promise](HttpResponsePtr response) {
            if (!response->ready) return;
            promise->set_value(std::move(response));
        };

        if (!http_request(std::move(request_ptr), std::move(callback))) {
            promise->set_exception(std::make_exception_ptr(std::runtime_error("Failed to add request to HttpRequestManager")));
        }

        return future;
    }

    /// \brief Sends an HTTP request with a specified host, path, and callback.
    /// \param method HTTP method (e.g., "GET", "POST").
    /// \param host Host name or IP address.
    /// \param path URL path for the request.
    /// \param query Query parameters for the request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for POST requests.
    /// \param callback Callback function to be executed upon request completion.
    /// \return True if the request was successfully added to the manager, false otherwise.
    bool http_request(
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

        request_ptr->set_url(host, path, query);
        request_ptr->method  = method;
        request_ptr->headers = headers;
        request_ptr->content = content;
        return http_request(std::move(request_ptr), std::move(callback));
    }

    /// \brief Sends an asynchronous HTTP GET request with a callback.
    /// \param url The full request URL.
    /// \param query Query parameters for the GET request.
    /// \param headers HTTP headers to include.
    /// \param callback Callback function to be executed upon request completion.
    /// \return True if the request was successfully added to the manager, false otherwise.
    bool http_get(
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
    /// \return A future containing the HttpResponsePtr with the response details.
    std::future<HttpResponsePtr> http_get(
            const std::string& url,
            const QueryParams& query,
            const Headers& headers) {

#       if __cplusplus >= 201402L
        auto request_ptr = std::make_unique<HttpRequest>();
#       else
        auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest());
#       endif

        request_ptr->set_url(url, query);
        request_ptr->method = "GET";
        request_ptr->headers = headers;

        return http_request(std::move(request_ptr));
    }

    /// \brief Sends an asynchronous HTTP POST request with a callback.
    /// \param url The full request URL.
    /// \param query Query parameters for the POST request.
    /// \param headers HTTP headers to include.
    /// \param content The body content for the POST request.
    /// \param callback Callback function to be executed upon request completion.
    /// \return True if the request was successfully added to the manager, false otherwise.
    bool http_post(
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
    /// \return A future containing the HttpResponsePtr with the response details.
    std::future<HttpResponsePtr> http_post(
            const std::string& url,
            const QueryParams& query,
            const Headers& headers,
            const std::string& content) {

#       if __cplusplus >= 201402L
        auto request_ptr = std::make_unique<HttpRequest>();
#       else
        auto request_ptr = std::unique_ptr<HttpRequest>(new HttpRequest());
#       endif

        request_ptr->set_url(url, query);
        request_ptr->method = "POST";
        request_ptr->headers = headers;
        request_ptr->content = content;

        return http_request(std::move(request_ptr));
    }

} // namespace kurlyk

#endif // _KURLYK_HTTP_UTILS_HPP_INCLUDED