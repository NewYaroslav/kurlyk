#pragma once
#ifndef _KURLYK_TYPES_ENUMS_HPP_INCLUDED
#define _KURLYK_TYPES_ENUMS_HPP_INCLUDED

/// \file enums.hpp
/// \brief Defines enums used across the Kurlyk library, including proxy types, rate limits, and WebSocket events.

namespace kurlyk {

    /// \enum ProxyType
    /// \brief Enumeration of supported proxy types compatible with libcurl.
    enum class ProxyType {
        PROXY_HTTP = 0,           ///< HTTP proxy.
        PROXY_HTTPS,              ///< HTTPS proxy.
        PROXY_HTTP_1_0,           ///< HTTP/1.0 proxy.
        PROXY_SOCKS4,             ///< SOCKS4 proxy.
        PROXY_SOCKS4A,            ///< SOCKS4A proxy.
        PROXY_SOCKS5,             ///< SOCKS5 proxy.
        PROXY_SOCKS5_HOSTNAME     ///< SOCKS5 proxy with hostname resolution.
    };

    /// \enum RateLimitType
    /// \brief Defines rate limit scope categories.
    enum class RateLimitType {
        RL_GENERAL,  ///< Applies globally to all requests.
        RL_SPECIFIC  ///< Applies to specific client/request.
    };

    /// \enum WebSocketEventType
    /// \brief Types of WebSocket events.
    enum class WebSocketEventType {
        WS_OPEN,     ///< Connection established.
        WS_MESSAGE,  ///< Message received.
        WS_CLOSE,    ///< Connection closed.
        WS_ERROR     ///< Error occurred.
    };

} // namespace kurlyk

#endif // _KURLYK_TYPES_ENUMS_HPP_INCLUDED