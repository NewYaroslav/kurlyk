#pragma once
#ifndef _KURLYK_HPP_INCLUDED
#define _KURLYK_HPP_INCLUDED

/// \file kurlyk.hpp
/// \brief Main header file for the Kurlyk library, providing HTTP and WebSocket support.

/// \def KURLYK_AUTO_INIT
/// \brief Enables automatic registration of managers during static initialization.
/// Define to 0 before including kurlyk headers to disable automatic registration.
#ifndef KURLYK_AUTO_INIT
#   define KURLYK_AUTO_INIT 1
#endif

/// \def KURLYK_AUTO_INIT_USE_ASYNC
/// \brief Determines whether the NetworkWorker runs in a background thread during automatic initialization.
/// Set to 1 to enable asynchronous execution, or 0 to require manual processing (synchronous mode).
/// Ignored if KURLYK_AUTO_INIT is not enabled.
#ifndef KURLYK_AUTO_INIT_USE_ASYNC
#   define KURLYK_AUTO_INIT_USE_ASYNC 1
#endif

/// \def KURLYK_HTTP_SUPPORT
/// \brief Enables HTTP request support in the Kurlyk library.
/// Set to 1 to enable, or 0 to disable HTTP functionality.
#ifndef KURLYK_HTTP_SUPPORT
#   define KURLYK_HTTP_SUPPORT 1
#endif

/// \def KURLYK_WEBSOCKET_SUPPORT
/// \brief Enables WebSocket support in the Kurlyk library.
/// Set to 1 to enable, or 0 to disable WebSocket functionality.
#ifndef KURLYK_WEBSOCKET_SUPPORT
#   define KURLYK_WEBSOCKET_SUPPORT 1
#endif

/// \def KURLYK_ENABLE_JSON
/// \brief Enables JSON serialization support for enums and types.
#ifndef KURLYK_ENABLE_JSON
#   define KURLYK_ENABLE_JSON 0
#endif

#ifdef __EMSCRIPTEN__
#   define KURLYK_USE_EMSCRIPTEN    ///< Defines the use of Emscripten-specific WebSocket handling.
#else
#   define KURLYK_USE_CURL          ///< Enables the use of libcurl for HTTP support on non-Emscripten platforms.
#   define KURLYK_USE_SIMPLEWEB     ///< Enables the use of Simple-WebSocket-Server for WebSocket support on non-Emscripten platforms.
#endif

#include "kurlyk/core.hpp"

#if KURLYK_HTTP_SUPPORT
#include "kurlyk/http.hpp"
#endif

#if KURLYK_WEBSOCKET_SUPPORT
#include "kurlyk/websocket.hpp"
#endif

#include "kurlyk/startup.hpp"

/// \namespace kurlyk
/// \brief Primary namespace for the Kurlyk library, encompassing initialization, request management, and utility functions.
namespace kurlyk {

} // namespace kurlyk

#endif // _KURLYK_HPP_INCLUDED
