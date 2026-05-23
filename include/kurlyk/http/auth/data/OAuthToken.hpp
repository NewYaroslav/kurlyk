#pragma once
#ifndef _KURLYK_HTTP_AUTH_DATA_OAUTH_TOKEN_HPP_INCLUDED
#define _KURLYK_HTTP_AUTH_DATA_OAUTH_TOKEN_HPP_INCLUDED

/// \file OAuthToken.hpp
/// \brief Defines the OAuthToken structure for storing OAuth2 token data.

#include <string>
#include <chrono>

#if KURLYK_JSON_SUPPORT
#   include <nlohmann/json.hpp>
#endif

namespace kurlyk {

    /// \struct OAuthToken
    /// \brief Holds the result of an OAuth2 token exchange.
    struct OAuthToken {
        std::string access_token;   ///< The access token string.
        std::string refresh_token;  ///< The refresh token string (may be empty).
        std::string token_type;     ///< Token type, typically "Bearer".
        std::string scope;          ///< Granted scope (may be empty).
        int64_t expires_at_ms = 0;  ///< Absolute expiration time in milliseconds since epoch (0 = unknown).
        std::string raw_response;   ///< Raw server response body for debugging.

        /// \brief Checks whether the token has expired.
        /// \param skew_ms Safety margin in milliseconds (default 60s) to account
        ///               for network latency and clock drift.
        /// \return `true` if the token has a known expiration and it is within
        ///         the skew margin of expiry.
        bool is_expired(int64_t skew_ms = 60000) const {
            if (expires_at_ms <= 0) return false;
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            return (now_ms + skew_ms) >= expires_at_ms;
        }
    };

} // namespace kurlyk

#if KURLYK_JSON_SUPPORT

namespace kurlyk {

    inline void to_json(nlohmann::json& j, const OAuthToken& t) {
        j = nlohmann::json{
            {"access_token", t.access_token},
            {"refresh_token", t.refresh_token},
            {"token_type", t.token_type},
            {"scope", t.scope},
            {"expires_at_ms", t.expires_at_ms}
        };
    }

    inline void from_json(const nlohmann::json& j, OAuthToken& t) {
        if (j.contains("access_token")) t.access_token = j["access_token"].get<std::string>();
        if (j.contains("refresh_token")) t.refresh_token = j["refresh_token"].get<std::string>();
        if (j.contains("token_type")) t.token_type = j["token_type"].get<std::string>();
        if (j.contains("scope")) t.scope = j["scope"].get<std::string>();
        if (j.contains("expires_at_ms")) t.expires_at_ms = j["expires_at_ms"].get<int64_t>();
    }

} // namespace kurlyk

#endif // KURLYK_JSON_SUPPORT

#endif // _KURLYK_HTTP_AUTH_DATA_OAUTH_TOKEN_HPP_INCLUDED
