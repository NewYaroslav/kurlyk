#pragma once
#ifndef _KURLYK_WEB_SOCKET_MANAGER_HPP_INCLUDED
#define _KURLYK_WEB_SOCKET_MANAGER_HPP_INCLUDED

/// \file WebSocketManager.hpp
/// \brief Defines the WebSocketManager class for managing WebSocket clients in a singleton pattern.

#ifdef KURLYK_USE_SIMPLEWEB
#include "Client/SimpleWeb/WebSocketClientAdapter.hpp"
#endif

namespace kurlyk {

    /// \class WebSocketManager
    /// \brief Manages WebSocket client instances, providing centralized control for processing, resetting, and lifecycle management.
    class WebSocketManager {
    public:

        /// \brief Get the singleton instance of WebSocketManager.
        /// \return Reference to the singleton instance of WebSocketManager.
        static WebSocketManager& get_instance() {
            static WebSocketManager* instance = new WebSocketManager();
            return *instance;
        }

        /// \brief Processes all active WebSocket clients managed by this instance.
        ///
        /// Iterates over each WebSocket client and calls its `process` method to handle pending operations.
        /// Automatically removes clients that are no longer active from the internal client list.
        void process() {
            std::lock_guard<std::mutex> lock(m_client_list_mutex);
            for (auto &&client_weak_ptr : m_client_list) {
                if (auto client_ptr = client_weak_ptr.lock()) {
                    client_ptr->process();
                }
            }
            // Remove expired clients from the list
            m_client_list.remove_if([](const std::weak_ptr<IWebSocketClient>& client_weak_ptr) {
                return client_weak_ptr.expired();
            });
        }

        /// \brief Shuts down all active WebSocket clients managed by this instance.
        ///
        /// Iterates over each WebSocket client and calls its `shutdown` method to stop and clear its state.
        void shutdown() {
            std::lock_guard<std::mutex> lock(m_client_list_mutex);
            for (auto &&client_weak_ptr : m_client_list) {
                if (auto client_ptr = client_weak_ptr.lock()) {
                    client_ptr->shutdown();
                }
            }
        }

        /// \brief Checks if any WebSocket client is currently running.
        /// \return True if at least one WebSocket client is in a running state, otherwise false.
        const bool is_loaded() const {
            std::lock_guard<std::mutex> lock(m_client_list_mutex);
            for (auto &&client_weak_ptr : m_client_list) {
                if (auto client_ptr = client_weak_ptr.lock()) {
                    if (client_ptr->is_running()) return true;
                }
            }
            return false;
        }

        /// \brief Creates and returns a new WebSocket client instance based on the platform defined by compilation flags.
        /// \return A shared pointer to the created IWebSocketClient instance.
        std::shared_ptr<IWebSocketClient> create_client() {
            std::shared_ptr<IWebSocketClient> client;

#           ifdef KURLYK_USE_EMSCRIPTEN
            // Client for the Emscripten platform
#           if __cplusplus >= 201402L
            client = std::make_shared<EmscriptenWebSocketClientAdapter>();
#           else
            client = std::shared_ptr<EmscriptenWebSocketClientAdapter>(new EmscriptenWebSocketClientAdapter());
#           endif
#           endif

#           ifdef KURLYK_USE_SIMPLEWEB
            // Client for other platforms
#           if __cplusplus >= 201402L
            client = std::make_shared<SimpleWebSocketClientAdapter>();
#           else
            client = std::shared_ptr<SimpleWebSocketClientAdapter>(new SimpleWebSocketClientAdapter());
#           endif
#           endif

            std::lock_guard<std::mutex> lock(m_client_list_mutex);
            m_client_list.push_back(client);
            return client;
        }

    private:
        mutable std::mutex                          m_client_list_mutex; ///< Mutex for synchronizing access to the client list.
        std::list<std::weak_ptr<IWebSocketClient>>  m_client_list;       ///< List of WebSocket clients managed by the WebSocketManager.

        /// \brief Private constructor to enforce singleton pattern.
        WebSocketManager() = default;

        /// \brief Private destructor to enforce singleton pattern.
        ~WebSocketManager() = default;

        /// \brief Deleted copy constructor to enforce the singleton pattern.
        WebSocketManager(const WebSocketManager&) = delete;

        /// \brief Deleted copy assignment operator to enforce the singleton pattern.
        WebSocketManager& operator=(const WebSocketManager&) = delete;

    }; // WebSocketManager

}

#endif // _KURLYK_WEB_SOCKET_MANAGER_HPP_INCLUDED
