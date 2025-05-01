#pragma once
#ifndef _KURLYK_TYPES_TYPE_UTILS_HPP_INCLUDED
#define _KURLYK_TYPES_TYPE_UTILS_HPP_INCLUDED

/// \file type_utils.hpp
/// \brief Provides utilities for enum-to-string conversion, parsing, JSON (if enabled), and stream output.

namespace kurlyk {

    /// \brief Converts a ProxyType enum value to its string representation.
    inline const std::string& to_str(ProxyType value) noexcept {
        static const std::vector<std::string> names = {
            "PROXY_HTTP", "PROXY_HTTPS", "PROXY_HTTP_1_0",
            "PROXY_SOCKS4", "PROXY_SOCKS4A", "PROXY_SOCKS5", "PROXY_SOCKS5_HOSTNAME"
        };
        return names[static_cast<size_t>(value)];
    }

    /// \brief Converts a RateLimitType enum value to its string representation.
    inline const std::string& to_str(RateLimitType value) noexcept {
        static const std::vector<std::string> names = {
            "RL_GENERAL", "RL_SPECIFIC"
        };
        return names[static_cast<size_t>(value)];
    }

    /// \brief Converts a WebSocketEventType enum value to its string representation.
    inline const std::string& to_str(WebSocketEventType value) noexcept {
        static const std::vector<std::string> names = {
            "WS_OPEN", "WS_MESSAGE", "WS_CLOSE", "WS_ERROR"
        };
        return names[static_cast<size_t>(value)];
    }

    /// \brief Template specialization to convert string to enum value.
    template <typename T>
    T to_enum(const std::string& str);

    template <>
    inline ProxyType to_enum<ProxyType>(const std::string& str) {
        static const std::unordered_map<std::string, ProxyType> map = {
            {"PROXY_HTTP", ProxyType::PROXY_HTTP},
            {"PROXY_HTTPS", ProxyType::PROXY_HTTPS},
            {"PROXY_HTTP_1_0", ProxyType::PROXY_HTTP_1_0},
            {"PROXY_SOCKS4", ProxyType::PROXY_SOCKS4},
            {"PROXY_SOCKS4A", ProxyType::PROXY_SOCKS4A},
            {"PROXY_SOCKS5", ProxyType::PROXY_SOCKS5},
            {"PROXY_SOCKS5_HOSTNAME", ProxyType::PROXY_SOCKS5_HOSTNAME}
        };
        auto it = map.find(utils::to_upper_case(str));
        if (it != map.end()) return it->second;
        throw std::invalid_argument("Invalid ProxyType: " + str);
    }

    template <>
    inline RateLimitType to_enum<RateLimitType>(const std::string& str) {
        static const std::unordered_map<std::string, RateLimitType> map = {
            {"RL_GENERAL", RateLimitType::RL_GENERAL},
            {"RL_SPECIFIC", RateLimitType::RL_SPECIFIC}
        };
        auto it = map.find(utils::to_upper_case(str));
        if (it != map.end()) return it->second;
        throw std::invalid_argument("Invalid RateLimitType: " + str);
    }

    template <>
    inline WebSocketEventType to_enum<WebSocketEventType>(const std::string& str) {
        static const std::unordered_map<std::string, WebSocketEventType> map = {
            {"WS_OPEN", WebSocketEventType::WS_OPEN},
            {"WS_MESSAGE", WebSocketEventType::WS_MESSAGE},
            {"WS_CLOSE", WebSocketEventType::WS_CLOSE},
            {"WS_ERROR", WebSocketEventType::WS_ERROR}
        };
        auto it = map.find(utils::to_upper_case(str));
        if (it != map.end()) return it->second;
        throw std::invalid_argument("Invalid WebSocketEventType: " + str);
    }
    
    inline std::ostream& operator<<(std::ostream& os, ProxyType type) {
        return os << to_str(type);
    }
    
    inline std::ostream& operator<<(std::ostream& os, RateLimitType type) {
        return os << to_str(type);
    }
    
    inline std::ostream& operator<<(std::ostream& os, WebSocketEventType type) {
        return os << to_str(type);
    }

#ifdef KURLYK_USE_JSON

    inline void to_json(nlohmann::json& j, const ProxyType& value) {
        j = to_str(value);
    }

    inline void from_json(const nlohmann::json& j, ProxyType& value) {
        value = to_enum<ProxyType>(j.get<std::string>());
    }

    inline void to_json(nlohmann::json& j, const RateLimitType& value) {
        j = to_str(value);
    }

    inline void from_json(const nlohmann::json& j, RateLimitType& value) {
        value = to_enum<RateLimitType>(j.get<std::string>());
    }

    inline void to_json(nlohmann::json& j, const WebSocketEventType& value) {
        j = to_str(value);
    }

    inline void from_json(const nlohmann::json& j, WebSocketEventType& value) {
        value = to_enum<WebSocketEventType>(j.get<std::string>());
    }

#endif // KURLYK_USE_JSON

#if KURLYK_HTTP_SUPPORT

    /// \brief Converts ProxyType to CURL proxy constant.
    /// \param type The ProxyType enum value.
    /// \return Corresponding CURL constant.
    inline long to_curl_proxy_type(ProxyType type) {
        static const long values[] = {
            CURLPROXY_HTTP,
            CURLPROXY_HTTPS,
            CURLPROXY_HTTP_1_0,
            CURLPROXY_SOCKS4,
            CURLPROXY_SOCKS4A,
            CURLPROXY_SOCKS5,
            CURLPROXY_SOCKS5_HOSTNAME
        };
        return values[static_cast<size_t>(type)];
    }

#endif

} // namespace kurlyk

#endif // _KURLYK_TYPES_TYPE_UTILS_HPP_INCLUDED
