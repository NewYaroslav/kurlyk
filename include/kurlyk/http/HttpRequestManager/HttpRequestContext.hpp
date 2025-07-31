#pragma once
#ifndef _KURLYK_HTTP_REQUEST_CONTEXT_HPP_INCLUDED
#define _KURLYK_HTTP_REQUEST_CONTEXT_HPP_INCLUDED

/// \file HttpRequestContext.hpp
/// \brief Defines the HttpRequestContext class for managing HTTP request context, including retries and timing.

namespace kurlyk {

    /// \class HttpRequestContext
    /// \brief Represents the context of an HTTP request, including the request object, callback function, retry attempts, and timing.
    class HttpRequestContext {
    public:
        using time_point_t = std::chrono::steady_clock::time_point;

        std::unique_ptr<HttpRequest> request;       ///< The HTTP request associated with this context.
        HttpResponseCallback         callback;      ///< Callback function to be invoked when the request completes.
        long                         retry_attempt; ///< Number of retry attempts made for this request.
        time_point_t                 start_time;    ///< Time when the request was initially created or last retried.

        /// \brief Constructs a HttpRequestContext with the specified request and callback.
        /// \param request_ptr A unique pointer to the HTTP request object.
        /// \param callback Callback function to be invoked upon request completion.
        HttpRequestContext(
            std::unique_ptr<HttpRequest> request_ptr,
            HttpResponseCallback callback)
            : request(std::move(request_ptr)),
              callback(std::move(callback)),
              retry_attempt(0) {
        }
        
        HttpRequestContext() = default;
    }; // HttpRequestContext

} // namespace kurlyk

#endif // _KURLYK_HTTP_REQUEST_CONTEXT_HPP_INCLUDED
