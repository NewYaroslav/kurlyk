#pragma once
#ifndef _KURLYK_HTTP_AUTH_DATA_OAUTH_CONFIG_HPP_INCLUDED
#define _KURLYK_HTTP_AUTH_DATA_OAUTH_CONFIG_HPP_INCLUDED

/// \file OAuthConfig.hpp
/// \brief Defines the OAuthConfig structure for OAuth2 client configuration.

#include <string>

#if KURLYK_ENABLE_JSON
#   include <nlohmann/json.hpp>
#endif

namespace kurlyk {

    /// \struct OAuthConfig
    /// \brief Stores client configuration for an OAuth2 Authorization Code + PKCE flow.
    struct OAuthConfig {
        std::string client_id;              ///< OAuth2 client identifier.
        std::string client_secret;          ///< Client secret (optional, usually empty for public/PKCE clients).
        std::string authorization_endpoint; ///< Authorization server URL.
        std::string token_endpoint;         ///< Token exchange URL.
        std::string redirect_uri;           ///< Registered redirect URI.
        std::string scope;                  ///< Space-separated requested scopes.
        bool use_pkce = true;               ///< Whether to use PKCE (RFC 7636). Default `true`.
        std::string audience;               ///< Optional audience parameter.
        std::string prompt;                 ///< Optional prompt parameter (e.g. "consent").
        std::string access_type;            ///< Optional access_type parameter (e.g. "offline").
    };

} // namespace kurlyk

#ifdef KURLYK_USE_JSON

namespace kurlyk {

    inline void to_json(nlohmann::json& j, const OAuthConfig& c) {
        j = nlohmann::json{
            {"client_id", c.client_id},
            {"client_secret", c.client_secret},
            {"authorization_endpoint", c.authorization_endpoint},
            {"token_endpoint", c.token_endpoint},
            {"redirect_uri", c.redirect_uri},
            {"scope", c.scope},
            {"use_pkce", c.use_pkce},
            {"audience", c.audience},
            {"prompt", c.prompt},
            {"access_type", c.access_type}
        };
    }

    inline void from_json(const nlohmann::json& j, OAuthConfig& c) {
        if (j.contains("client_id")) c.client_id = j["client_id"].get<std::string>();
        if (j.contains("client_secret")) c.client_secret = j["client_secret"].get<std::string>();
        if (j.contains("authorization_endpoint")) c.authorization_endpoint = j["authorization_endpoint"].get<std::string>();
        if (j.contains("token_endpoint")) c.token_endpoint = j["token_endpoint"].get<std::string>();
        if (j.contains("redirect_uri")) c.redirect_uri = j["redirect_uri"].get<std::string>();
        if (j.contains("scope")) c.scope = j["scope"].get<std::string>();
        if (j.contains("use_pkce")) c.use_pkce = j["use_pkce"].get<bool>();
        if (j.contains("audience")) c.audience = j["audience"].get<std::string>();
        if (j.contains("prompt")) c.prompt = j["prompt"].get<std::string>();
        if (j.contains("access_type")) c.access_type = j["access_type"].get<std::string>();
    }

} // namespace kurlyk

#endif // KURLYK_USE_JSON

#endif // _KURLYK_HTTP_AUTH_DATA_OAUTH_CONFIG_HPP_INCLUDED
