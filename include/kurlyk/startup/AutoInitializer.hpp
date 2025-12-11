#pragma once
#ifndef _KURLYK_STARTUP_AUTO_INITIALIZER_HPP_INCLUDED
#define _KURLYK_STARTUP_AUTO_INITIALIZER_HPP_INCLUDED

/// \file AutoInitializer.hpp
/// \brief Provides automatic initialization and shutdown for the Kurlyk network system.

namespace kurlyk::startup {

    /// \class AutoInitializer
    /// \brief Automatically registers and manages network task managers.
    ///
    /// This class ensures that managers like HttpRequestManager and WebSocketManager are
    /// registered to the NetworkWorker during construction, and properly shut down in
    /// reverse order during destruction.
    class AutoInitializer {
    public:
        /// \brief Constructs and registers all available managers.
        AutoInitializer() {
            auto &instance = core::NetworkWorker::get_instance();
#           if KURLYK_HTTP_SUPPORT
            m_http = &HttpRequestManager::get_instance();
            instance.register_manager(m_http);
#           endif
#           if KURLYK_WEBSOCKET_SUPPORT
            m_ws = &WebSocketManager::get_instance();
            instance.register_manager(m_ws);
#           endif
            instance.start(KURLYK_AUTO_INIT_USE_ASYNC);
        }

        /// \brief Shuts down the NetworkWorker before program termination.
        ~AutoInitializer() {
            core::NetworkWorker::get_instance().shutdown();
        }

    private:
#       if KURLYK_HTTP_SUPPORT
        HttpRequestManager* m_http = nullptr;
#       endif
#       if KURLYK_WEBSOCKET_SUPPORT
        WebSocketManager*   m_ws = nullptr;
#       endif
    };
	
	inline AutoInitializer _kurlyk_auto_initializer;

} // namespace kurlyk::startup

#endif // _KURLYK_STARTUP_AUTO_INITIALIZER_HPP_INCLUDED
