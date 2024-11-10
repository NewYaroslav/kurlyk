#pragma once
#ifndef _KURLYK_HPP_INCLUDED
#define _KURLYK_HPP_INCLUDED

/// \file kurlyk.hpp
/// \brief Main header file for the Kurlyk library, providing HTTP and WebSocket support.

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

#ifdef __EMSCRIPTEN__
#   define KURLYK_USE_EMSCRIPTEN    ///< Defines the use of Emscripten-specific WebSocket handling.
#else
#   define KURLYK_USE_CURL          ///< Enables the use of libcurl for HTTP support on non-Emscripten platforms.
#   define KURLYK_USE_SIMPLEWEB     ///< Enables the use of Simple-WebSocket-Server for WebSocket support on non-Emscripten platforms.
#endif

#include "parts/NetworkWorker.hpp"

#if KURLYK_HTTP_SUPPORT
#include "parts/Http/Client.hpp"
#include "parts/Http/Utils.hpp"
#endif

#if KURLYK_WEBSOCKET_SUPPORT
#include "parts/WebSocket/Client.hpp"
#endif

/// \namespace kurlyk
/// \brief Primary namespace for the Kurlyk library, encompassing initialization, request management, and utility functions.
namespace kurlyk {

    /// \brief Initializes the Kurlyk library, setting up necessary managers and the network worker.
    /// \param use_async If true, enables asynchronous processing for requests.
    /// Call this function before using the library to ensure all components are initialized.
    void init(const bool use_async = true) {
        HttpRequestManager::get_instance();
        WebSocketManager::get_instance();
        NetworkWorker::get_instance().start(use_async);
    }

    /// \brief Deinitializes the Kurlyk library, stopping the network worker and releasing resources.
    /// Call this function to clean up resources before exiting the application.
    void deinit() {
        NetworkWorker::get_instance().stop();
    }

    /// \brief Processes pending requests (used in synchronous mode).
    /// This function should be called periodically if the library is used in synchronous mode.
    void process() {
        NetworkWorker::get_instance().process();
    }

    /// \brief Shuts down all network operations, resetting the state of the network worker and clearing pending requests.
    /// Use this function to stop all network operations and prepare the library for shutdown.
    void shutdown() {
        NetworkWorker::get_instance().shutdown();
    }

} // namespace kurlyk

#endif // _KURLYK_HPP_INCLUDED
