#pragma once
#ifndef _KURLYK_STARTUP_RUNTIME_HPP_INCLUDED
#define _KURLYK_STARTUP_RUNTIME_HPP_INCLUDED

/// \file runtime.hpp
/// \brief Provides manual initialization and shutdown functions for the Kurlyk network system.

namespace kurlyk {

	/// \brief Initializes the Kurlyk library, setting up necessary managers and the network worker.
    /// \param use_async If true, enables asynchronous processing for requests.
    /// Call this function before using the library to ensure all components are initialized.
    inline void init(const bool use_async = true) {
        auto &instance = core::NetworkWorker::get_instance();
#       if KURLYK_HTTP_SUPPORT
        instance.register_manager(&HttpRequestManager::get_instance());
#       endif
#       if KURLYK_WEBSOCKET_SUPPORT
        instance.register_manager(&WebSocketManager::get_instance());
#       endif
        instance.start(use_async);
    }

    /// \brief Deinitializes the Kurlyk library, stopping the network worker and releasing resources.
    /// Call this function to clean up resources before exiting the application.
    inline void deinit() {
        core::NetworkWorker::get_instance().stop();
    }

    /// \brief Processes pending requests (used in synchronous mode).
    /// This function should be called periodically if the library is used in synchronous mode.
    inline void process() {
        core::NetworkWorker::get_instance().process();
    }

	/// \brief Shuts down all network operations, resetting the state of the network worker and clearing pending requests.
    /// Use this function to stop all network operations and prepare the library for shutdown.
    inline void shutdown() {
        core::NetworkWorker::get_instance().shutdown();
    }

} // namespace kurlyk

#endif // _KURLYK_STARTUP_RUNTIME_HPP_INCLUDED
