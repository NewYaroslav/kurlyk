#pragma once
#ifndef _KURLYK_BASE_WEB_SOCKET_CLIENT_HPP_INCLUDED
#define _KURLYK_BASE_WEB_SOCKET_CLIENT_HPP_INCLUDED

/// \file BaseWebSocketClient.hpp
/// \brief Contains the definition of the BaseWebSocketClient class, which provides the base functionality for WebSocket clients.

#include "BaseWebSocketClient/IWebSocketSender.hpp"
#include "BaseWebSocketClient/WebSocketEventData.hpp"
#include "BaseWebSocketClient/IWebSocketClient.hpp"
#include "BaseWebSocketClient/WebSocketRateLimiter.hpp"

namespace kurlyk {

    /// \class BaseWebSocketClient
    /// \brief Base class implementing core functionality for WebSocket clients, managing events, states, and message sending.
    class BaseWebSocketClient : public IWebSocketClient, public std::enable_shared_from_this<BaseWebSocketClient> {
    public:

        /// \brief Default constructor.
        BaseWebSocketClient() = default;

        /// \brief Virtual destructor.
        virtual ~BaseWebSocketClient() = default;

        /// \brief Accessor for the event handler function.
        /// This method provides access to the event handler function used for handling WebSocket events. The returned
        /// reference allows getting or setting the function that will be called when a WebSocket event occurs.
        /// \return A reference to a `std::function<void(std::unique_ptr<WebSocketEventData>)>` representing the event handler function.
        std::function<void(std::unique_ptr<WebSocketEventData>)>& event_handler() override final {
            return m_on_event;
        }

        /// \brief Accesses the notification handler for WebSocket events.
        ///
        /// The notification handler triggers the network worker to process WebSocket events
        /// by notifying it that an event has occurred, typically through `NetworkWorker::get_instance().notify()`.
        /// \return A reference to the notification handler callback function.
        std::function<void()>& notify_handler() override final {
            return m_on_event_notify;
        }

        /// \brief Sets the configuration for the WebSocket client.
        /// \param config A unique pointer to the WebSocket configuration object.
        /// \param callback Callback function to be executed upon configuration completion.
        void set_config(std::unique_ptr<WebSocketConfig> config, std::function<void(bool)> callback) override final {
            m_fsm_event_queue.push_event(FSMEventData(FsmEvent::UpdateConfig, std::move(config), std::move(callback)));
        }

        /// \brief Initiates a connection to the WebSocket server.
        /// \param callback Callback function to be executed upon connection completion, receiving a success status.
        void connect(std::function<void(bool)> callback) override final {
            m_fsm_event_queue.push_event(FSMEventData(FsmEvent::RequestConnect, std::move(callback)));
        }

        /// \brief Closes the connection to the WebSocket server.
        /// \param callback Callback function to be executed upon disconnection completion, receiving a success status.
        void disconnect(std::function<void(bool)> callback) override final {
            m_fsm_event_queue.push_event(FSMEventData(FsmEvent::RequestDisconnect, std::move(callback)));
        }

        /// \brief Checks if the WebSocket client is actively running.
        /// \return True if the client is running, otherwise false.
        bool is_connected() const override final {
            return m_is_connected;
        }

        /// \brief Checks if the WebSocket client is actively running.
        /// \return True if the client is running, false otherwise.
        bool is_running() const override final {
            return m_is_running || m_fsm_event_queue.has_events();
        }

        /// \brief Retrieves all pending WebSocket events in a batch.
        ///
        /// This method enables efficient processing of multiple accumulated events at once.
        /// \return A list of unique pointers to `WebSocketEventData` objects representing the events.
        std::list<std::unique_ptr<WebSocketEventData>> receive_events() const override final {
            std::lock_guard<std::mutex> lock(m_event_queue_mutex);
            auto event_queue = std::move(m_event_queue);
            m_event_queue.clear();
            return event_queue;
        }

        /// \brief Retrieves the next available WebSocket event, if any.
        ///
        /// This method supports event-by-event processing by returning the next event in the queue.
        /// \return A unique pointer to a `WebSocketEventData` object representing an event, or nullptr if no events are available.
        std::unique_ptr<WebSocketEventData> receive_event() const override final {
            std::lock_guard<std::mutex> lock(m_event_queue_mutex);
            if (m_event_queue.empty()) return nullptr;
            std::unique_ptr<WebSocketEventData> event = std::move(*m_event_queue.begin());
            m_event_queue.erase(m_event_queue.begin());
            return event;
        }

        /// \brief Send a message through the WebSocket.
        /// \param message The message to send.
        /// \param rate_limit_id The rate limit type to apply.
        /// \param callback The callback to be invoked after sending.
        /// \return True if the message was successfully queued, false otherwise.
        bool send_message(
                const std::string &message,
                long rate_limit_id,
                std::function<void(const std::error_code& ec)> callback = nullptr) override final {
            if (message.empty() || !is_connected()) return false;
            std::lock_guard<std::mutex> lock(m_message_queue_mutex);
#           if __cplusplus >= 201402L
            m_message_queue.push_back(std::make_shared<WebSocketSendInfo>(message, rate_limit_id, false, 0, std::move(callback)));
#           else
            m_message_queue.push_back(std::shared_ptr<WebSocketSendInfo>(new WebSocketSendInfo(message, rate_limit_id, false, 0, std::move(callback))));
#           endif
            return true;
        }

        /// \brief Send a close request through the WebSocket.
        /// \param status The status code to send with the close request.
        /// \param reason The reason for closing the connection.
        /// \param callback The callback to be invoked after sending.
        /// \return True if the close request was successfully queued, false otherwise.
        bool send_close(
                const int status = 1000,
                const std::string &reason = std::string(),
                std::function<void(const std::error_code& ec)> callback = nullptr) override final {
            if (!is_connected()) return false;
            std::lock_guard<std::mutex> lock(m_message_queue_mutex);
#           if __cplusplus >= 201402L
            m_message_queue.push_back(std::make_shared<WebSocketSendInfo>(reason, 0, true, status, std::move(callback)));
#           else
            m_message_queue.push_back(std::shared_ptr<WebSocketSendInfo>(new WebSocketSendInfo(reason, 0, true, status, std::move(callback))));
#           endif
            return true;
        }


        /// \brief Processes internal operations such as event handling and state updates.
        ///
        /// This function should be called periodically to ensure timely processing of internal state changes,
        /// events, and messages.
        void process() override final {
            process_fsm_state();
            process_message_queue();
            process_send_callback_queue();
        }

        /// \brief Shuts down the WebSocket client, disconnecting and clearing all pending events.
        /// Initiates a disconnect event and processes any remaining events until the client stops running.
        void shutdown() override final {
            m_fsm_event_queue.push_event(FSMEventData(FsmEvent::RequestDisconnect));
            while (is_running()) {
                process_fsm_state();
                process_send_callback_queue();
            }
        }

    protected:

        std::unique_ptr<WebSocketConfig> m_config; ///< Current configuration for the WebSocket.

        /// \brief Finite State Machine (FSM) states for the WebSocket connection.
        enum class FsmState {
            INIT,               ///< Initialization state.
            CONNECTING,         ///< Awaiting connection.
            WORKING,            ///< Connection active and working.
            RECONNECTING,       ///< Reconnection attempt.
            STOPPED             ///< Stopped state.
        } m_fsm_state = FsmState::INIT;

        /// \brief Represents events in the finite state machine.
        enum class FsmEvent {
            RequestConnect,     ///< Request to connect.
            RequestDisconnect,  ///< Request to disconnect.
            ConnectionOpened,   ///< Connection opened successfully.
            ConnectionClosed,   ///< Connection closed.
            ConnectionError,    ///< Error in connection.
            MessageReceived,    ///< Incoming WebSocket message.
            UpdateConfig        ///< Update configuration.
        };


        /// \brief Initializes the WebSocket connection. Must be implemented in derived classes.
        virtual bool init_websocket() = 0;

        /// \brief Deinitializes the WebSocket connection. Must be implemented in derived classes.
        virtual void deinit_websocket() = 0;

        /// \brief Sends a WebSocket message.
        /// \param send_info Reference to WebSocketSendInfo containing message details.
        virtual void send_message(std::shared_ptr<WebSocketSendInfo>& send_info) = 0;

        /// \brief Sends a close request.
        /// \param send_info Reference to WebSocketSendInfo containing close details.
        virtual void send_close(std::shared_ptr<WebSocketSendInfo>& send_info) = 0;

        /// \brief Creates a generic WebSocket event.
        /// \return Unique pointer to the created WebSocketEventData.
        std::unique_ptr<WebSocketEventData> create_websocket_event() {
#           if __cplusplus >= 201402L
            auto event = std::make_unique<WebSocketEventData>();
#           else
            auto event = std::unique_ptr<WebSocketEventData>(new WebSocketEventData());
#           endif
            event->sender = shared_from_this();
            return event;
        }

        /// \brief Creates a WebSocket close event with a specified reason and status code.
        /// This method generates a WebSocket close event, setting the event type to "Close" and
        /// including an optional reason and status code.
        /// \param reason The reason for the closure. Defaults to "Normal Closure."
        /// \param status_code The status code associated with the closure. Defaults to 1000 (normal closure).
        /// \return Unique pointer to a WebSocketEventData representing the close event.
        std::unique_ptr<WebSocketEventData> create_websocket_close_event(
                const std::string& reason = "Normal Closure",
                int status_code = 1000) {
            auto websocket_event = create_websocket_event();
            websocket_event->event_type = WebSocketEventType::WS_CLOSE;
            websocket_event->message = reason;
            websocket_event->status_code = status_code;
            return websocket_event;
        }

        /// \brief Creates a WebSocket error event with a specified error code.
        /// This method generates a WebSocket error event, setting the event type to "Error"
        /// and associating it with the provided error code.
        /// \param error_code The error code representing the nature of the error.
        /// \return Unique pointer to a WebSocketEventData representing the error event.
        std::unique_ptr<WebSocketEventData> create_websocket_error_event(
                const std::error_code& error_code) {
            auto websocket_event = create_websocket_event();
            websocket_event->event_type = WebSocketEventType::WS_ERROR;
            websocket_event->error_code = error_code;
            return websocket_event;
        }

        /// \brief Adds a send callback to the queue.
        /// \param error_code The error code returned after the send operation.
        /// \param callback The callback function to be called with the error code.
        void add_send_callback(
                const std::error_code& error_code,
                const std::function<void(const std::error_code& ec)> &callback) {
            std::lock_guard<std::mutex> lock(m_send_callback_queue_mutex);
            m_send_callback_queue.push_back(std::make_pair(error_code, callback));
        }

        /// \brief Adds an FSM event to the event queue and triggers the notify handler.
        /// \param event_type The event type to be pushed.
        /// \param event_data The event data to be associated with the event.
        void add_fsm_event(FsmEvent event_type, std::unique_ptr<WebSocketEventData> event_data) {
            m_fsm_event_queue.push_event(FSMEventData(event_type, std::move(event_data)));
            m_on_event_notify();
        }

    private:

        std::function<void(std::unique_ptr<WebSocketEventData>)> m_on_event; ///< Function to handle WebSocket events. Called when a new event is received.
        std::function<void()> m_on_event_notify; ///< Function to notify about new events in the FSM.

        /// \struct FSMEventData
        /// \brief Represents an event in the finite state machine (FSM) with optional associated data and callback.
        struct FSMEventData {
            FsmEvent                            event_type;  ///< The type of the FSM event.
            std::unique_ptr<WebSocketEventData> event_data;  ///< Optional WebSocket event data associated with the FSM event.
            std::unique_ptr<WebSocketConfig>    config_data; ///< Optional configuration data for FSM settings.
            std::function<void(bool)>           callback;    ///< Optional callback function to execute on event completion.

            /// \brief Move constructor for FSMEventData.
            /// Transfers ownership of the event data, configuration data, and callback from another FSMEventData instance.
            FSMEventData(FSMEventData&& other) noexcept
                : event_type(other.event_type),
                  event_data(std::move(other.event_data)),
                  config_data(std::move(other.config_data)),
                  callback(std::move(other.callback)) {
            }

            /// \brief Move assignment operator for FSMEventData.
            /// Transfers ownership of the event data, configuration data, and callback from another FSMEventData instance.
            /// \return A reference to this FSMEventData instance.
            FSMEventData& operator=(FSMEventData&& other) noexcept {
                if (this != &other) {
                    event_type = other.event_type;
                    event_data = std::move(other.event_data);
                    config_data = std::move(other.config_data);
                    callback = std::move(other.callback);
                }
                return *this;
            }

            /// \brief Deleted copy constructor to prevent copying.
            FSMEventData(const FSMEventData&) = delete;

            /// \brief Deleted copy assignment operator to prevent copying.
            FSMEventData& operator=(const FSMEventData&) = delete;

            /// \brief Constructs FSMEventData with an event type and WebSocket event data.
            /// \param event_type The type of the FSM event.
            /// \param event_data Unique pointer to the WebSocket event data.
            FSMEventData(
                    FsmEvent event_type,
                    std::unique_ptr<WebSocketEventData> &&event_data) :
                event_type(event_type),
                event_data(std::move(event_data)) {
            }

            /// \brief Constructs FSMEventData with an event type, configuration data, and a callback.
            /// \param event_type The type of the FSM event.
            /// \param config_data Unique pointer to the configuration data.
            /// \param callback Callback function to be executed on event completion.
            FSMEventData(
                    FsmEvent event_type,
                    std::unique_ptr<WebSocketConfig> &&config_data,
                    std::function<void(bool)> &&callback) :
                event_type(event_type),
                config_data(std::move(config_data)),
                callback(std::move(callback)) {
            }

            /// \brief Constructs FSMEventData with an event type and a callback.
            /// \param event_type The type of the FSM event.
            /// \param callback Callback function to be executed on event completion.
            FSMEventData(
                    FsmEvent event_type,
                    std::function<void(bool)> &&callback) :
                event_type(event_type),
                callback(std::move(callback)) {
            }

            /// \brief Constructs FSMEventData with only an event type.
            /// \param event_type The type of the FSM event.
            FSMEventData(FsmEvent event_type) :
                event_type(event_type) {
            }

        }; // FSMEventData

        utils::EventQueue<FSMEventData>         m_fsm_event_queue;          ///< Queue for FSM events, managing the event sequence for the FSM.
        long                                    m_reconnect_attempt = 0;    ///< Counter for the number of reconnection attempts.
        std::atomic<bool>                       m_is_running = ATOMIC_VAR_INIT(false);  ///< Atomic flag indicating if the client is running.
        std::atomic<bool>                       m_is_connected = ATOMIC_VAR_INIT(false);///< Atomic flag indicating if the client is connected.

        WebSocketRateLimiter                    m_rate_limiter;             ///< Rate limiter for controlling the frequency of message sending.
        std::chrono::steady_clock::time_point   m_close_time;               ///< Timestamp of the last WebSocket close event, used for reconnection timing.

        mutable std::mutex                      m_event_queue_mutex;        ///< Mutex for synchronizing access to the event queue.
        using event_data_ptr_t  = std::unique_ptr<WebSocketEventData>;      ///< Alias for unique pointers to WebSocketEventData.
        mutable std::list<event_data_ptr_t>     m_event_queue;              ///< Queue holding pending WebSocket events.

        std::mutex                              m_message_queue_mutex;      ///< Mutex for synchronizing access to the message queue.
        using send_info_ptr_t   = std::shared_ptr<WebSocketSendInfo>;       ///< Alias for shared pointers to WebSocketSendInfo.
        std::list<send_info_ptr_t>              m_message_queue;            ///< Queue holding messages to be sent over the WebSocket.

        std::mutex                              m_send_callback_queue_mutex;///< Mutex for synchronizing access to the send callback queue.
        using send_callback_t   = std::pair<std::error_code, std::function<void(const std::error_code& ec)>>; ///< Alias for callback pairs with error codes.
        std::list<send_callback_t>              m_send_callback_queue;      ///< Queue holding send callbacks with their respective error codes.

        /// \brief Processes the current FSM state and transitions to the appropriate next state.
        void process_fsm_state() {
            switch (m_fsm_state) {
            case FsmState::INIT:
                process_state_init();
                break;
            case FsmState::CONNECTING:
                process_state_connecting();
                break;
            case FsmState::WORKING:
                process_state_working();
                break;
            case FsmState::RECONNECTING:
                process_state_reconnecting();
                break;
            case FsmState::STOPPED:
                process_state_stopped();
                break;
            };
        }

        /// \brief Handles the INIT state. Initializes connection or updates configuration.
        void process_state_init() {
            if (!m_fsm_event_queue.has_events()) return;

            auto event = m_fsm_event_queue.pop_event();
            switch (event.event_type) {
            case FsmEvent::RequestConnect:
                if (!m_config) {
                    handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                    if (event.callback) event.callback(false);
                    m_fsm_state = FsmState::STOPPED;
                    break;
                }
                if (!init_websocket()) {
                    handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                    if (event.callback) event.callback(false);
                    m_fsm_state = FsmState::STOPPED;
                    break;
                }
                m_is_running = true;
                if (event.callback) event.callback(true);
                m_fsm_state = FsmState::CONNECTING;
                break;
            case FsmEvent::UpdateConfig:
                m_config = std::move(event.config_data);
                if (m_config) {
                    m_rate_limiter.set_limit(m_config->rate_limits);
                    if (event.callback) event.callback(true);
                } else {
                    if (event.callback) event.callback(false);
                }
                break;
            default:
                if (event.callback) event.callback(false);
                break;
            };
        }

        /// \brief Handles the CONNECTING state. Manages connection attempt, errors, or disconnection.
        void process_state_connecting() {
            if (!m_fsm_event_queue.has_events()) return;

            auto event = m_fsm_event_queue.pop_event();
            switch (event.event_type) {
            case FsmEvent::ConnectionOpened:
                handle_open_event(std::move(event.event_data));
                m_reconnect_attempt = 0;
                m_is_running = true;
                m_fsm_state = FsmState::WORKING;
                break;
            case FsmEvent::ConnectionError:
                handle_error_event(std::move(event.event_data));
            case FsmEvent::ConnectionClosed:
                deinit_websocket();
                if (event.event_type == FsmEvent::ConnectionClosed) {
                    handle_close_event(std::move(event.event_data));
                } else {
                    handle_close_event(create_websocket_close_event("Going Away", 1001));
                }

                if (!m_config->reconnect) {
                    m_is_running = false;
                    m_fsm_state = FsmState::INIT;
                    break;
                }

                m_reconnect_attempt++;
                m_close_time = std::chrono::steady_clock::now();
                m_is_running = true;
                m_fsm_state = FsmState::RECONNECTING;
                break;
            case FsmEvent::RequestDisconnect: {
                deinit_websocket();
                handle_close_event();

                m_reconnect_attempt = 0;
                m_is_running = false;
                if (event.callback) event.callback(true);
                m_fsm_state = FsmState::INIT;
                break;
            }
            case FsmEvent::UpdateConfig:
                deinit_websocket();
                handle_close_event();

                m_config = std::move(event.config_data);
                if (!m_config) {
                    handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                    if (event.callback) event.callback(false);
                    m_fsm_state = FsmState::STOPPED;
                    break;
                }
                m_rate_limiter.set_limit(m_config->rate_limits);

                m_reconnect_attempt = 0;
                if (!init_websocket()) {
                    handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                    if (event.callback) event.callback(false);
                    m_fsm_state = FsmState::STOPPED;
                    break;
                }

                m_is_running = true;
                if (event.callback) event.callback(true);
                m_fsm_state = FsmState::CONNECTING;
                break;
            default:
                if (event.callback) event.callback(false);
                break;
            };
        }

        /// \brief Handles the WORKING state. Processes incoming events and manages connection health.
        void process_state_working() {
            bool is_message;
            while (m_fsm_event_queue.has_events()) {
                is_message = false;
                auto event = m_fsm_event_queue.pop_event();
                switch (event.event_type) {
                case FsmEvent::RequestDisconnect:
                    deinit_websocket();
                    handle_close_event();

                    m_reconnect_attempt = 0;
                    m_is_running = false;
                    if (event.callback) event.callback(true);
                    m_fsm_state = FsmState::INIT;
                    break;
                case FsmEvent::ConnectionError:
                    handle_error_event(std::move(event.event_data));
                case FsmEvent::ConnectionClosed:
                    deinit_websocket();
                    if (event.event_type == FsmEvent::ConnectionClosed) {
                        handle_close_event(std::move(event.event_data));
                    } else {
                        handle_close_event(create_websocket_close_event("Going Away", 1001));
                    }

                    if (!m_config->reconnect) {
                        m_is_running = false;
                        m_fsm_state = FsmState::INIT;
                        break;
                    }

                    m_reconnect_attempt++;
                    m_close_time = std::chrono::steady_clock::now();
                    m_is_running = true;
                    m_fsm_state = FsmState::RECONNECTING;
                    break;
                case FsmEvent::UpdateConfig:
                    deinit_websocket();
                    handle_close_event();

                    m_config = std::move(event.config_data);
                    if (!m_config) {
                        handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                        if (event.callback) event.callback(false);
                        m_fsm_state = FsmState::STOPPED;
                        break;
                    }
                    m_rate_limiter.set_limit(m_config->rate_limits);

                    m_reconnect_attempt = 0;
                    if (!init_websocket()) {
                        handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                        if (event.callback) event.callback(false);
                        m_fsm_state = FsmState::STOPPED;
                        break;
                    }

                    m_is_running = true;
                    if (event.callback) event.callback(true);
                    m_fsm_state = FsmState::CONNECTING;
                    break;
                case FsmEvent::MessageReceived:
                    handle_message_event(std::move(event.event_data));
                    is_message = true;
                    break;
                default:
                    if (event.callback) event.callback(false);
                    break;
                };
                if (!is_message) break;
            }
        }

        /// \brief Handles the RECONNECTING state. Attempts to reconnect based on configuration settings.
        void process_state_reconnecting() {
            if (m_fsm_event_queue.has_events()){
                auto event = m_fsm_event_queue.pop_event();
                switch (event.event_type) {
                case FsmEvent::RequestDisconnect:
                    m_is_running = false;
                    if (event.callback) event.callback(true);
                    m_fsm_state = FsmState::INIT;
                    break;
                case FsmEvent::UpdateConfig:
                    m_config = std::move(event.config_data);
                    if (!m_config) {
                        handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                        if (event.callback) event.callback(false);
                        m_fsm_state = FsmState::STOPPED;
                        break;
                    }
                    m_rate_limiter.set_limit(m_config->rate_limits);

                    m_reconnect_attempt = 0;
                    if (!init_websocket()) {
                        handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                        if (event.callback) event.callback(false);
                        m_fsm_state = FsmState::STOPPED;
                        break;
                    }

                    m_is_running = true;
                    if (event.callback) event.callback(true);
                    m_fsm_state = FsmState::CONNECTING;
                    break;
                case FsmEvent::MessageReceived:
                    handle_message_event(std::move(event.event_data));
                    break;
                default:
                    if (event.callback) event.callback(false);
                    break;
                };
            }

            if (!m_config) {
                handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                m_fsm_state = FsmState::STOPPED;
                return;
            }

            if (m_config->reconnect) {
                if (m_config->reconnect_attempts &&
                    m_reconnect_attempt >= m_config->reconnect_attempts) {
                    m_is_running = false;
                    m_fsm_state = FsmState::INIT;
                    return;
                }

                auto now = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_close_time);
                if (duration.count() >= m_config->reconnect_delay) {
                    if (!init_websocket()) {
                        handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                        m_fsm_state = FsmState::STOPPED;
                        return;
                    }
                    m_is_running = true;
                    m_fsm_state = FsmState::CONNECTING;
                }
                return;
            }

            m_is_running = false;
            m_fsm_state = FsmState::INIT;
        }

        /// \brief Processes the STOPPED state in the FSM.
        void process_state_stopped() {
            if (!m_fsm_event_queue.has_events()) return;

            auto event = m_fsm_event_queue.pop_event();
            switch (event.event_type) {
            case FsmEvent::RequestConnect:
                if (!m_config) {
                    handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                    if (event.callback) event.callback(false);
                    m_fsm_state = FsmState::STOPPED;
                    break;
                }
                if (!init_websocket()) {
                    handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                    if (event.callback) event.callback(false);
                    m_fsm_state = FsmState::STOPPED;
                    break;
                }
                m_is_running = true;
                if (event.callback) event.callback(true);
                m_fsm_state = FsmState::CONNECTING;
                break;
            case FsmEvent::UpdateConfig:
                m_config = std::move(event.config_data);
                if (!m_config) {
                    handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                    if (event.callback) event.callback(false);
                    m_fsm_state = FsmState::STOPPED;
                    break;
                }
                m_rate_limiter.set_limit(m_config->rate_limits);

                m_reconnect_attempt = 0;
                if (!init_websocket()) {
                    handle_error_event(utils::make_error_code(utils::ClientError::InvalidConfiguration));
                    if (event.callback) event.callback(false);
                    m_fsm_state = FsmState::STOPPED;
                    break;
                }

                m_is_running = true;
                if (event.callback) event.callback(true);
                m_fsm_state = FsmState::CONNECTING;
                break;
            default:
                if (event.callback) event.callback(false);
                break;
            };
        }

        /// \brief Processes the queue of messages to be sent over the WebSocket.
        /// Filters the message queue according to the rate limiter. Messages allowed by the rate limiter are
        /// transferred to a temporary list and sent sequentially. Close requests are sent immediately.
        void process_message_queue() {
            std::unique_lock<std::mutex> lock(m_message_queue_mutex);
            if (m_message_queue.empty()) return;

            std::list<send_info_ptr_t> message_queue;
            auto it = m_message_queue.begin();
            while (it != m_message_queue.end()) {
                auto& send_info = *it;
                // Check if the message is allowed by the rate limiter.
                if (!m_rate_limiter.allow_request(send_info->rate_limit_id)) {
                    ++it;
                    continue;
                }
                message_queue.push_back(std::move(send_info));
                it = m_message_queue.erase(it);
            }
            lock.unlock();
            if (message_queue.empty()) return;

            for (auto &send_info : message_queue) {
                if (!send_info->is_send_close) {
                    send_message(send_info);
                    continue;
                }
                send_close(send_info);
            }
        }

        /// \brief Processes the queue of send callbacks.
        /// For each callback in the queue, it calls the callback function with the associated error code.
        /// This method is typically called after a message has been sent or an error has occurred.
        void process_send_callback_queue() {
            std::unique_lock<std::mutex> lock(m_send_callback_queue_mutex);
            if (m_send_callback_queue.empty()) return;
            auto send_callback_queue = std::move(m_send_callback_queue);
            m_send_callback_queue.clear();

            for (auto &item : send_callback_queue) {
                item.second(item.first);
            }
        }

        /// \brief Handles the event when the WebSocket connection is opened.
        /// Sets the connection state to connected and triggers the event handler, if set.
        /// Otherwise, stores the event in the event queue.
        /// \param event Unique pointer to the WebSocket open event data.
        void handle_open_event(std::unique_ptr<WebSocketEventData> event) {
            if (!m_is_connected) {
                m_is_connected = true;
                if (m_on_event) {
                    m_on_event(std::move(event));
                } else {
                    std::lock_guard<std::mutex> lock(m_event_queue_mutex);
                    m_event_queue.push_back(std::move(event));
                }
            }
        }

        /// \brief Handles the event when the WebSocket connection is closed.
        /// Sets the connection state to disconnected and triggers the event handler, if set.
        /// Otherwise, stores the event in the event queue. Generates a close event if none is provided.
        /// \param event Unique pointer to the WebSocket close event data. Defaults to nullptr.
        void handle_close_event(std::unique_ptr<WebSocketEventData> event = nullptr) {
            if (!event) {
                event = create_websocket_close_event();
            }
            if (m_is_connected) {
                m_is_connected = false;
                if (m_on_event) {
                    m_on_event(std::move(event));
                } else {
                    std::lock_guard<std::mutex> lock(m_event_queue_mutex);
                    m_event_queue.push_back(std::move(event));
                }
            }
        }

        /// \brief Handles WebSocket error events and queues them if no event handler is set.
        /// If an event handler exists, it directly processes the error event.
        /// \param event Unique pointer to the WebSocket error event data.
        void handle_error_event(std::unique_ptr<WebSocketEventData> event) {
            if (m_on_event) {
                m_on_event(std::move(event));
                return;
            }
            std::lock_guard<std::mutex> lock(m_event_queue_mutex);
            m_event_queue.push_back(std::move(event));
        }

        /// \brief Overloaded method to handle WebSocket error events using an error code.
        /// \param error_code The error code representing the WebSocket error.
        void handle_error_event(const std::error_code& error_code) {
            handle_error_event(create_websocket_error_event(error_code));
        }

        /// \brief Handles incoming WebSocket message events and queues them if no event handler is set.
        /// If an event handler exists, it directly processes the message event.
        /// \param event Unique pointer to the WebSocket message event data.
        void handle_message_event(std::unique_ptr<WebSocketEventData> event) {
            if (m_on_event) {
                m_on_event(std::move(event));
                return;
            }
            std::lock_guard<std::mutex> lock(m_event_queue_mutex);
            m_event_queue.push_back(std::move(event));
        }

    }; // BaseWebSocketClient

}; // namespace kurlyk


#endif // _KURLYK_BASE_WEB_SOCKET_CLIENT_HPP_INCLUDED
