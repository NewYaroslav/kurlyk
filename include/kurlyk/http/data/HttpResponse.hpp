#pragma once
#ifndef KURLYK_HEADER_KURLYK_HTTP_DATA_HTTP_RESPONSE_HPP_INCLUDED
#define KURLYK_HEADER_KURLYK_HTTP_DATA_HTTP_RESPONSE_HPP_INCLUDED

/// \file HttpResponse.hpp
/// \brief Defines the HttpResponse class and related HTTP response types.

namespace kurlyk {

    /// \class HttpResponse
    /// \brief Represents an HTTP response.
    ///
    /// HttpResponse stores response headers, body, status, errors,
    /// retry metadata, and timing metrics.
    class HttpResponse {
    public:
        Headers         headers;            ///< HTTP response headers.
        std::string     content;            ///< Body content of the HTTP response.
        std::error_code error_code;         ///< Error code indicating response or transport issues, if any.
        std::string     error_message;      ///< Error message describing the issue, if any.
        long            status_code = 0;    ///< HTTP status code of the response (e.g., 200, 404).
        long            retry_attempt = 0;  ///< Number of retry attempts performed for this request.
        bool            ready = false;      ///< Indicates whether the response is final and ready to be processed.
        bool            stream_chunk = false; ///< Indicates whether content contains an intermediate streaming body chunk.
        
        // --- Timing metrics (all values in seconds) ---
        double namelookup_time    = -1; ///< Time until name resolution completed (DNS).
        double connect_time       = -1; ///< Time until TCP connection established.
        double appconnect_time    = -1; ///< Time until SSL handshake completed (HTTPS only).
        double pretransfer_time   = -1; ///< Time until request is ready to be sent.
        double starttransfer_time = -1; ///< Time until first byte is received from the server.
        double total_time         = -1; ///< Total time of the transfer.
    }; // HttpResponse

    /// \brief Owning pointer to an HTTP response.
    using HttpResponsePtr = std::unique_ptr<HttpResponse>;

    /// \brief Callback invoked with an HTTP response.
    /// \param response Owning pointer to the HTTP response.
    using HttpResponseCallback = std::function<void(HttpResponsePtr response)>;

} // namespace kurlyk

#endif // KURLYK_HEADER_KURLYK_HTTP_DATA_HTTP_RESPONSE_HPP_INCLUDED

