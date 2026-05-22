#pragma once
#ifndef _KURLYK_HTTP_AUTH_HPP_INCLUDED
#define _KURLYK_HTTP_AUTH_HPP_INCLUDED

/// \file auth.hpp
/// \brief Aggregates HTTP authentication providers and OAuth2 PKCE client.

#include "auth/IAuthProvider.hpp"
#include "auth/BearerTokenAuthProvider.hpp"
#include "auth/ApiKeyAuthProvider.hpp"
#include "auth/TokenStorage.hpp"
#include "auth/OAuthPkceClient.hpp"

#endif // _KURLYK_HTTP_AUTH_HPP_INCLUDED
