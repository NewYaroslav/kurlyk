#pragma once
#ifndef _KURLYK_I_WEB_SOCKET_CLIENT_HPP_INCLUDED
#define _KURLYK_I_WEB_SOCKET_CLIENT_HPP_INCLUDED

/// \file IWebSocketClient.hpp
/// \brief Defines an interface for WebSocket client functionality, including connection management, event handling, and configuration.

#include "IWebSocketSender.hpp"
#include "../Components/Config.hpp"
#include "../Components/EventData.hpp"
#include <functional>
#include <list>
#include <memory>

namespace kurlyk {

    /// \class IWebSocketClient
    /// \brief Interface for a WebSocket client, providing methods for connection management, configuration, and event handling.
    class IWebSocketClient : public IWebSocketSender {
    public:

        /// \brief Default constructor.
        IWebSocketClient() = default;

        /// \brief Virtual destructor for safe cleanup in derived classes.
        virtual ~IWebSocketClient() = default;

        /// \brief Accessor for the event handler function.
        ///
        /// This method provides access to the event handler function used for handling WebSocket events. The returned
        /// reference allows getting or setting the function that will be called when a WebSocket event occurs.
        /// \return A reference to a `std::function<void(std::unique_ptr<WebSocketEventData>)>` representing the event handler function.
        virtual std::function<void(std::unique_ptr<WebSocketEventData>)>& event_handler() = 0;

        /// \brief Accesses the notification handler for WebSocket events.
        ///
        /// The notification handler triggers the network worker to process WebSocket events
        /// by notifying it that an event has occurred, typically through `NetworkWorker::get_instance().notify()`.
        /// \return A reference to the notification handler callback function.
        virtual std::function<void()>& notify_handler() = 0;

        /// \brief Sets the configuration for the WebSocket client.
        /// \param config A unique pointer to the WebSocket configuration object.
        /// \param callback Callback function to be executed upon configuration completion.
        virtual void set_config(
            std::unique_ptr<WebSocketConfig> config,
            std::function<void(bool)> callback) = 0;

        /// \brief Initiates a connection to the WebSocket server.
        /// \param callback Callback function to be executed upon connection completion, receiving a success status.
        virtual void connect(std::function<void(bool)> callback) = 0;

        /// \brief Closes the connection to the WebSocket server.
        /// \param callback Callback function to be executed upon disconnection completion, receiving a success status.
        virtual void disconnect(std::function<void(bool)> callback) = 0;

        /// \brief Checks if the WebSocket client is actively running.
        /// \return True if the client is running, otherwise false.
        virtual bool is_running() const = 0;

        /// \brief Retrieves all pending WebSocket events in a batch.
        ///
        /// This method enables efficient processing of multiple accumulated events at once.
        /// \return A list of unique pointers to `WebSocketEventData` objects representing the events.
        virtual std::list<std::unique_ptr<WebSocketEventData>> receive_events() const = 0;

        /// \brief Retrieves the next available WebSocket event, if any.
        ///
        /// This method supports event-by-event processing by returning the next event in the queue.
        /// \return A unique pointer to a `WebSocketEventData` object representing an event, or nullptr if no events are available.
        virtual std::unique_ptr<WebSocketEventData> receive_event() const = 0;

        /// \brief Processes internal operations such as event handling and state updates.
        ///
        /// This function should be called periodically to ensure timely processing of internal state changes,
        /// events, and messages.
        virtual void process() = 0;

        /// \brief Shuts down the WebSocket client, disconnecting and clearing all pending events.
        /// Initiates a disconnect event and processes any remaining events until the client stops running.
        virtual void shutdown() = 0;

    }; // IWebSocketClient

    /// \brief Alias for a shared pointer to an IWebSocketClient instance.
    using WebSocketClientPtr = std::shared_ptr<IWebSocketClient>;

} // namespace kurlyk

#endif // _KURLYK_I_WEB_SOCKET_CLIENT_HPP_INCLUDED
