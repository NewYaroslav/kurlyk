#pragma once
#ifndef _KURLYK_HTTP_AUTH_TOKEN_STORAGE_HPP_INCLUDED
#define _KURLYK_HTTP_AUTH_TOKEN_STORAGE_HPP_INCLUDED

/// \file TokenStorage.hpp
/// \brief Defines the ITokenStorage interface for persisting and retrieving OAuth tokens.

#include "../data/OAuthToken.hpp"
#include <string>

namespace kurlyk {
namespace http {
namespace auth {

    /// \class ITokenStorage
    /// \brief Abstract interface for token persistence.
    ///
    /// Implementations may store tokens in memory, files, encrypted keychains,
    /// or platform-specific credential vaults. This interface does not prescribe
    /// encryption — that is the responsibility of the concrete implementation.
    class ITokenStorage {
    public:
        virtual ~ITokenStorage() {}

        /// \brief Saves an OAuth token.
        /// \param token The token to persist.
        /// \return `true` on success.
        virtual bool save(const OAuthToken& token) = 0;

        /// \brief Loads an OAuth token.
        /// \param out_token Receives the loaded token.
        /// \return `true` if a token was successfully loaded.
        virtual bool load(OAuthToken& out_token) = 0;

        /// \brief Clears any stored token.
        /// \return `true` on success.
        virtual bool clear() = 0;
    };

} // namespace auth
} // namespace http
} // namespace kurlyk

#endif // _KURLYK_HTTP_AUTH_TOKEN_STORAGE_HPP_INCLUDED
