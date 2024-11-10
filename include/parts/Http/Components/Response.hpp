#pragma once
#ifndef _KURLYK_HTTP_RESPONSE_HPP_INCLUDED
#define _KURLYK_HTTP_RESPONSE_HPP_INCLUDED

/// \file Response.hpp
/// \brief Defines the HttpResponse class and related types for handling HTTP responses.

#include "../../Utils.hpp"
#include <functional>
#include <memory>

namespace kurlyk {

    /// \class HttpResponse
    /// \brief Represents the response received from an HTTP request, including headers, content, and status.
    class HttpResponse {
    public:
        Headers         headers;            ///< HTTP response headers.
        std::string     content;            ///< The body content of the HTTP response.
        std::error_code error_code;         ///< Error code indicating issues with the response, if any.
        std::string     error_message;      ///< Error message detailing the issue, if any.
        long            status_code = 0;    ///< HTTP status code of the response (e.g., 200, 404).
        long            retry_attempt = 0;  ///<
        bool            ready = false;      ///< Indicates if the response is ready to be processed.
    }; // HttpResponse

    /// \brief A unique pointer to an HttpResponse object for memory management.
    using HttpResponsePtr = std::unique_ptr<HttpResponse>;

    /// \brief Type definition for the callback function used to handle HTTP responses.
    /// \param response A pointer to the HttpResponse object.
    using HttpResponseCallback = std::function<void(HttpResponsePtr response)>;

} // namespace kurlyk

#endif // _KURLYK_HTTP_RESPONSE_HPP_INCLUDED

