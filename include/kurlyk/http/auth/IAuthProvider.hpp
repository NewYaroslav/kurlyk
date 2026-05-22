#pragma once
#ifndef _KURLYK_HTTP_AUTH_IAUTH_PROVIDER_HPP_INCLUDED
#define _KURLYK_HTTP_AUTH_IAUTH_PROVIDER_HPP_INCLUDED

/// \file IAuthProvider.hpp
/// \brief Defines the IAuthProvider interface for HTTP authentication strategies.

#include "../data/HttpRequest.hpp"
#include "kurlyk/utils/CaseInsensitiveMultimap.hpp"

namespace kurlyk {
namespace http {
namespace auth {

    /// \class IAuthProvider
    /// \brief Interface for authentication providers that modify HTTP requests or headers.
    class IAuthProvider {
    public:
        virtual ~IAuthProvider() {}

        /// \brief Modifies an HttpRequest in-place to include authentication credentials.
        /// \param request The HTTP request to authorize.
        /// \return `true` if authorization was applied successfully.
        virtual bool authorize(HttpRequest& request) const = 0;

        /// \brief Modifies a header map in-place to include authentication credentials.
        /// \param headers The headers to authorize.
        /// \return `true` if authorization was applied successfully.
        virtual bool authorize(Headers& headers) const = 0;
    };

} // namespace auth
} // namespace http
} // namespace kurlyk

#endif // _KURLYK_HTTP_AUTH_IAUTH_PROVIDER_HPP_INCLUDED
