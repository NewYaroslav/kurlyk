#pragma once
#ifndef _KURLYK_IWEBSOCKET_SENDER_HPP_INCLUDED
#define _KURLYK_IWEBSOCKET_SENDER_HPP_INCLUDED

/// \file IWebSocketSender.hpp
/// \brief Defines the interface for a WebSocket sender, providing methods for sending messages and managing connection state.

namespace kurlyk {

    /// \class IWebSocketSender
    /// \brief Interface for a WebSocket sender, offering methods to send messages, close connections, and check connection status.
    class IWebSocketSender {
    public:

        /// \brief Default constructor for IWebSocketSender.
        IWebSocketSender() = default;

        /// \brief Virtual destructor for IWebSocketSender.
        virtual ~IWebSocketSender() = default;

        /// \brief Retrieves the HTTP version used in the WebSocket connection.
        /// \return The HTTP version string.
        virtual std::string get_http_version() = 0;

        /// \brief Retrieves the headers associated with the WebSocket connection.
        /// \return A Headers object containing the HTTP headers.
        virtual Headers get_headers() = 0;

        /// \brief Retrieves the remote endpoint information.
        /// \return The remote endpoint as a string in the format "IP:Port".
        virtual std::string get_remote_endpoint() = 0;

        /// \brief Sends a WebSocket message.
        /// \param message The content of the message to be sent.
        /// \param rate_limit_id The ID of the rate limit to apply to this message. A value of 0 indicates the default or no rate limit.
        /// \param callback Optional callback to execute once the message is sent, providing an error code if any issues occur.
        /// \return True if the message was accepted for sending, false otherwise.
        virtual bool send_message(
                const std::string &message,
                long rate_limit_id = 0,
                std::function<void(const std::error_code&)> callback = nullptr) = 0;

        /// \brief Sends a close request to the WebSocket server.
        /// \param status The status code for the close request, default is 1000 (normal closure).
        /// \param reason Optional reason for closing the connection.
        /// \param callback Optional callback to execute once the close request is sent, providing an error code if any issues occur.
        /// \return True if the close request was accepted, false otherwise.
        virtual bool send_close(
                int status = 1000,
                const std::string &reason = std::string(),
                std::function<void(const std::error_code&)> callback = nullptr) = 0;

        /// \brief Checks if the WebSocket connection is currently active.
        /// \return True if the WebSocket is connected, false otherwise.
        virtual bool is_connected() const = 0;

    };

    /// \brief Alias for a shared pointer to an IWebSocketSender instance.
    using WebSocketSenderPtr = std::shared_ptr<IWebSocketSender>;

} // namespace kurlyk

#endif // _KURLYK_IWEBSOCKET_SENDER_HPP_INCLUDED
