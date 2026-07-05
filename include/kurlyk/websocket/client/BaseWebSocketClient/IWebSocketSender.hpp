#pragma once
#ifndef KURLYK_HEADER_KURLYK_WEBSOCKET_CLIENT_BASE_WEB_SOCKET_CLIENT_I_WEB_SOCKET_SENDER_HPP_INCLUDED
#define KURLYK_HEADER_KURLYK_WEBSOCKET_CLIENT_BASE_WEB_SOCKET_CLIENT_I_WEB_SOCKET_SENDER_HPP_INCLUDED

/// \file IWebSocketSender.hpp
/// \brief Defines the public sender abstraction exposed through WebSocket events.

namespace kurlyk {

    /// \class IWebSocketSender
    /// \brief Public sender abstraction used by WebSocketEventData::sender.
    /// Allows event handlers to inspect connection metadata and send follow-up messages without knowing the backend type.
    class IWebSocketSender {
    public:

        /// \brief Default constructor for IWebSocketSender.
        IWebSocketSender() = default;

        /// \brief Virtual destructor for safe cleanup through the sender abstraction.
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

        /// \brief Attempts to submit a WebSocket message and reports the admission result.
        /// \param message The content of the message to be sent.
        /// \param rate_limit_id The ID of the rate limit to apply to this message. A value of 0 indicates the default or no rate limit.
        /// \param callback Optional callback to execute once the message is sent, providing an error code if any issues occur.
        /// \return SubmitResult describing whether the message was accepted for sending.
        virtual SubmitResult submit_message(
                const std::string &message,
                long rate_limit_id = 0,
                std::function<void(const std::error_code&)> callback = nullptr) {
            const bool accepted = send_message(message, rate_limit_id, std::move(callback));
            return SubmitResult{accepted, std::error_code()};
        }

        /// \brief Sends a close request to the WebSocket server.
        /// \param status The status code for the close request, default is 1000 (normal closure).
        /// \param reason Optional reason for closing the connection.
        /// \param callback Optional callback to execute once the close request is sent, providing an error code if any issues occur.
        /// \return True if the close request was accepted, false otherwise.
        virtual bool send_close(
                int status = 1000,
                const std::string &reason = std::string(),
                std::function<void(const std::error_code&)> callback = nullptr) = 0;

        /// \brief Attempts to submit a close request and reports the admission result.
        /// \param status The status code for the close request, default is 1000 (normal closure).
        /// \param reason Optional reason for closing the connection.
        /// \param callback Optional callback to execute once the close request is sent, providing an error code if any issues occur.
        /// \return SubmitResult describing whether the close request was accepted.
        virtual SubmitResult submit_close(
                int status = 1000,
                const std::string &reason = std::string(),
                std::function<void(const std::error_code&)> callback = nullptr) {
            const bool accepted = send_close(status, reason, std::move(callback));
            return SubmitResult{accepted, std::error_code()};
        }

        /// \brief Checks if the WebSocket connection is currently active.
        /// \return True if the WebSocket is connected, false otherwise.
        virtual bool is_connected() const = 0;

    };

    /// \brief Shared pointer alias for the sender abstraction used in WebSocketEventData.
    using WebSocketSenderPtr = std::shared_ptr<IWebSocketSender>;

} // namespace kurlyk

#endif // KURLYK_HEADER_KURLYK_WEBSOCKET_CLIENT_BASE_WEB_SOCKET_CLIENT_I_WEB_SOCKET_SENDER_HPP_INCLUDED
