#pragma once
#ifndef _KURLYK_WEBSOCKET_SEND_INFO_HPP_INCLUDED
#define _KURLYK_WEBSOCKET_SEND_INFO_HPP_INCLUDED

/// \file WebSocketSendInfo.hpp
/// \brief Contains the WebSocketSendInfo class, which holds the context for sending WebSocket messages.

namespace kurlyk {

    /// \class WebSocketSendInfo
    /// \brief Holds information for sending a WebSocket message, including rate limiting, close status, and a callback.
    class WebSocketSendInfo {
    public:
        std::string message;        ///< Content of the WebSocket message to be sent.
        long rate_limit_id = 0;     ///< Rate limit ID applied to the message. A value of 0 implies the default rate limit or no limit if unspecified.
        bool is_send_close = false; ///< Indicates if this message is a close request.
        int status = 1000;          ///< Status code for the close request, default is normal closure (1000).
        std::function<void(const std::error_code&)> callback; ///< Callback invoked after sending, with error status.

        /// \brief Constructs a WebSocketSendInfo instance with the specified parameters.
        /// \param message Content of the WebSocket message to be sent.
        /// \param rate_limit_id ID for rate limiting the message. A value of 0 applies the default rate limit, or no limit if unspecified.
        /// \param is_send_close True if the message is a close request, false otherwise.
        /// \param status Status code for the close request; default is 1000 (normal closure).
        /// \param callback Callback function to be called upon completion of the send operation.
        WebSocketSendInfo(
                std::string message,
                long rate_limit_id = 0,
                bool is_send_close = false,
                int status = 1000,
                std::function<void(const std::error_code&)> callback = nullptr) :
            message(std::move(message)),
            rate_limit_id(rate_limit_id),
            is_send_close(is_send_close),
            status(status),
            callback(std::move(callback)) {}
    };

} // namespace kurlyk

#endif // _KURLYK_WEBSOCKET_SEND_INFO_HPP_INCLUDED
