#pragma once
#ifndef _KURLYK_HTTP_AUTH_API_KEY_AUTH_PROVIDER_HPP_INCLUDED
#define _KURLYK_HTTP_AUTH_API_KEY_AUTH_PROVIDER_HPP_INCLUDED

/// \file ApiKeyAuthProvider.hpp
/// \brief Provides API key authentication via header or query parameter.

#include "IAuthProvider.hpp"
#include "kurlyk/utils/percent_encoding.hpp"
#include <string>

namespace kurlyk {
namespace http {
namespace auth {

    /// \enum ApiKeyPlacement
    /// \brief Determines where the API key is attached.
    enum class ApiKeyPlacement {
        HEADER, ///< Sent as a custom HTTP header.
        QUERY   ///< Appended as a query parameter to the request URL.
    };

    /// \class ApiKeyAuthProvider
    /// \brief Injects an API key into the request either as a header or query parameter.
    class ApiKeyAuthProvider : public IAuthProvider {
    public:
        /// \brief Constructs a provider with a key name, value, and placement.
        /// \param key_name The header name or query parameter key.
        /// \param key_value The API key value.
        /// \param placement Where to place the key (header or query).
        ApiKeyAuthProvider(
                const std::string& key_name,
                const std::string& key_value,
                ApiKeyPlacement placement = ApiKeyPlacement::HEADER)
            : m_key_name(key_name)
            , m_key_value(key_value)
            , m_placement(placement) {}

        /// \brief Updates the stored API key value.
        /// \param key_value The new API key.
        void set_key_value(const std::string& key_value) {
            m_key_value = key_value;
        }

        bool authorize(HttpRequest& request) const override {
            if (m_placement == ApiKeyPlacement::HEADER) {
                request.headers.erase(m_key_name);
                request.headers.emplace(m_key_name, m_key_value);
                return true;
            }

            // QUERY placement
            const std::string encoded_key = utils::percent_encode(m_key_name);
            const std::string encoded_value = utils::percent_encode(m_key_value);
            const std::string param = encoded_key + "=" + encoded_value;

            if (request.url.find('?') == std::string::npos) {
                request.url += "?" + param;
            } else {
                request.url += "&" + param;
            }
            return true;
        }

        bool authorize(Headers& headers) const override {
            if (m_placement != ApiKeyPlacement::HEADER) {
                return false;
            }
            headers.erase(m_key_name);
            headers.emplace(m_key_name, m_key_value);
            return true;
        }

    private:
        std::string m_key_name;
        std::string m_key_value;
        ApiKeyPlacement m_placement;
    };

} // namespace auth
} // namespace http
} // namespace kurlyk

#endif // _KURLYK_HTTP_AUTH_API_KEY_AUTH_PROVIDER_HPP_INCLUDED
