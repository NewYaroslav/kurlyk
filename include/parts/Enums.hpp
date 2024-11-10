#pragma once
#ifndef _KURLYK_ENUMS_HPP_INCLUDED
#define _KURLYK_ENUMS_HPP_INCLUDED

/// \file Enums.hpp
/// \brief Defines enumerations used throughout the Kurlyk library, including proxy, rate limit, and WebSocket event types.

#include <curl/curl.h> // Include CURL headers for proxy type values

namespace kurlyk {

    /// \enum ProxyType
    /// \brief Enumeration of proxy types compatible with CURL.
    enum class ProxyType {
        HTTP = 0,           ///< HTTP proxy type.
        HTTPS,              ///< HTTPS proxy type.
        HTTP_1_0,           ///< HTTP 1.0 proxy type.
        SOCKS4,             ///< SOCKS4 proxy type.
        SOCKS4A,            ///< SOCKS4A proxy type.
        SOCKS5,             ///< SOCKS5 proxy type.
        SOCKS5_HOSTNAME     ///< SOCKS5 with hostname resolution.
    };

    /// \enum RateLimitType
    /// \brief Enumeration for rate limiting categories, either general or specific.
    enum class RateLimitType {
        General, ///< Applies rate limit to all requests.
        Specific ///< Applies rate limit to specific requests or contexts.
    };

    /// \enum WebSocketEventType
    /// \brief Enumeration for WebSocket events, indicating the type of activity on the connection.
    enum class WebSocketEventType {
        Open,       ///< Connection opened.
        Message,    ///< Message received.
        Close,      ///< Connection closed.
        Error       ///< Error occurred on WebSocket.
    };

    /// \brief Converts a ProxyType enum value to a CURL-compatible proxy type.
    /// \param type ProxyType enum value.
    /// \return Corresponding CURL proxy type.
    inline long to_curl_proxy_type(ProxyType type) {
        static const long data[] = {
            CURLPROXY_HTTP,
            CURLPROXY_HTTPS,
            CURLPROXY_HTTP_1_0,
            CURLPROXY_SOCKS4,
            CURLPROXY_SOCKS4A,
            CURLPROXY_SOCKS5,
            CURLPROXY_SOCKS5_HOSTNAME
        };
        return data[static_cast<size_t>(type)];
    }

} // namespace kurlyk

#endif // _KURLYK_ENUMS_HPP_INCLUDED
