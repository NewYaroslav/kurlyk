#pragma once
#ifndef KURLYK_HEADER_KURLYK_HTTP_AUTH_DATA_AUTH_RESULT_HPP_INCLUDED
#define KURLYK_HEADER_KURLYK_HTTP_AUTH_DATA_AUTH_RESULT_HPP_INCLUDED

/// \file AuthResult.hpp
/// \brief Defines authentication result types and error codes.

#include <string>
#include "OAuthToken.hpp"

#if KURLYK_JSON_SUPPORT
#   include <nlohmann/json.hpp>
#endif

namespace kurlyk {

    /// \enum AuthError
    /// \brief Classification of authentication errors.
    enum class AuthError {
        None,            ///< No error; operation succeeded.
        InvalidConfig,   ///< Missing or invalid client configuration.
        HttpError,       ///< HTTP request failed (non-2xx status or transport error).
        InvalidResponse, ///< Server response could not be parsed.
        TokenExpired,    ///< The token has expired and refresh failed.
        UnsupportedFlow, ///< The requested OAuth flow is not supported.
        StateMismatch    ///< Returned state parameter does not match expected value.
    };

    /// \struct AuthResult
    /// \brief Encapsulates the outcome of an authentication operation.
    struct AuthResult {
        bool success = false;           ///< Whether the operation succeeded.
        AuthError error = AuthError::None; ///< Error classification.
        std::string error_message;      ///< Human-readable error description (if any).
        OAuthToken token;               ///< Token data on success (may be partially filled on failure).
        std::string raw_response;       ///< Raw server response body for diagnostics.
    };

} // namespace kurlyk

#if KURLYK_JSON_SUPPORT

namespace kurlyk {

    inline void to_json(nlohmann::json& j, const AuthError& e) {
        switch (e) {
            case AuthError::None: j = "None"; break;
            case AuthError::InvalidConfig: j = "InvalidConfig"; break;
            case AuthError::HttpError: j = "HttpError"; break;
            case AuthError::InvalidResponse: j = "InvalidResponse"; break;
            case AuthError::TokenExpired: j = "TokenExpired"; break;
            case AuthError::UnsupportedFlow: j = "UnsupportedFlow"; break;
            case AuthError::StateMismatch: j = "StateMismatch"; break;
            default: j = "Unknown"; break;
        }
    }

    inline void from_json(const nlohmann::json& j, AuthError& e) {
        const std::string s = j.get<std::string>();
        if (s == "None") e = AuthError::None;
        else if (s == "InvalidConfig") e = AuthError::InvalidConfig;
        else if (s == "HttpError") e = AuthError::HttpError;
        else if (s == "InvalidResponse") e = AuthError::InvalidResponse;
        else if (s == "TokenExpired") e = AuthError::TokenExpired;
        else if (s == "UnsupportedFlow") e = AuthError::UnsupportedFlow;
        else if (s == "StateMismatch") e = AuthError::StateMismatch;
        else e = AuthError::None;
    }

    inline void to_json(nlohmann::json& j, const AuthResult& r) {
        j = nlohmann::json{
            {"success", r.success},
            {"error", r.error},
            {"error_message", r.error_message},
            {"token", r.token},
            {"raw_response", r.raw_response}
        };
    }

    inline void from_json(const nlohmann::json& j, AuthResult& r) {
        if (j.contains("success")) r.success = j["success"].get<bool>();
        if (j.contains("error")) r.error = j["error"].get<AuthError>();
        if (j.contains("error_message")) r.error_message = j["error_message"].get<std::string>();
        if (j.contains("token")) r.token = j["token"].get<OAuthToken>();
        if (j.contains("raw_response")) r.raw_response = j["raw_response"].get<std::string>();
    }

} // namespace kurlyk

#endif // KURLYK_JSON_SUPPORT

#endif // KURLYK_HEADER_KURLYK_HTTP_AUTH_DATA_AUTH_RESULT_HPP_INCLUDED
