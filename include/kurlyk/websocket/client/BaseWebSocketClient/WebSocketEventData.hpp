#pragma once
#ifndef _KURLYK_WEBSOCKET_EVENT_DATA_HPP_INCLUDED
#define _KURLYK_WEBSOCKET_EVENT_DATA_HPP_INCLUDED

/// \file WebSocketEventData.hpp
/// \brief Defines the WebSocketEventData class, which encapsulates data related to WebSocket events.

namespace kurlyk {

    /// \class WebSocketEventData
    /// \brief Encapsulates data for a WebSocket event, providing information about event type, message, status, and errors.
    ///
    /// This class is used to represent events that occur within the WebSocket lifecycle, such as
    /// connection open, message received, connection close, or errors.
    class WebSocketEventData {
    public:
        WebSocketEventType  event_type;    ///< Type of the WebSocket event, defining what occurred (e.g., Open, Close, Message, Error).
        std::string         message;       ///< The message content associated with the event, if any.
        long                status_code;   ///< Status code related to the event, set during the `Open` event (e.g., HTTP status code).
        std::error_code     error_code;    ///< Error code for the `Error` event, if an error occurred.
        WebSocketSenderPtr  sender;        ///< Pointer to the WebSocket sender, allowing further actions related to this event.
    }; // WebSocketEventData

}; // namespace kurlyk

#endif // _KURLYK_WEBSOCKET_EVENT_DATA_HPP_INCLUDED
