#pragma once
#ifndef KURLYK_HEADER_KURLYK_HTTP_AUTH_BEARER_TOKEN_AUTH_PROVIDER_HPP_INCLUDED
#define KURLYK_HEADER_KURLYK_HTTP_AUTH_BEARER_TOKEN_AUTH_PROVIDER_HPP_INCLUDED

/// \file BearerTokenAuthProvider.hpp
/// \brief Provides Bearer token authentication for HTTP requests.

#include "IAuthProvider.hpp"
#include <string>

namespace kurlyk {
namespace http {
namespace auth {

    /// \class BearerTokenAuthProvider
    /// \brief Injects an `Authorization: Bearer <token>` header.
    class BearerTokenAuthProvider : public IAuthProvider {
    public:
        /// \brief Constructs a provider with a Bearer token.
        /// \param token The bearer token string.
        explicit BearerTokenAuthProvider(const std::string& token)
            : m_token(token) {}

        /// \brief Sets a new Bearer token, replacing any previous value.
        /// \param token The new bearer token.
        void set_token(const std::string& token) {
            m_token = token;
        }

        bool authorize(HttpRequest& request) const override {
            if (m_token.empty()) return false;
            request.headers.erase("Authorization");
            request.headers.emplace("Authorization", "Bearer " + m_token);
            return true;
        }

        bool authorize(Headers& headers) const override {
            if (m_token.empty()) return false;
            headers.erase("Authorization");
            headers.emplace("Authorization", "Bearer " + m_token);
            return true;
        }

    private:
        std::string m_token;
    };

} // namespace auth
} // namespace http
} // namespace kurlyk

#endif // KURLYK_HEADER_KURLYK_HTTP_AUTH_BEARER_TOKEN_AUTH_PROVIDER_HPP_INCLUDED
