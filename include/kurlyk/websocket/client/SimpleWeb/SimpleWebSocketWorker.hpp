#pragma once
#ifndef _KURLYK_SIMPLE_WEB_SOCKET_WORKER_HPP_INCLUDED
#define _KURLYK_SIMPLE_WEB_SOCKET_WORKER_HPP_INCLUDED

/// \file SimpleWebSocketWorker.hpp
/// \brief Defines the SimpleWebSocketWorker class for managing WebSocket operations in a separate thread.

namespace kurlyk {

    /// \class SimpleWebSocketWorker
    /// \brief Singleton worker that manages asynchronous WebSocket operations.
    class SimpleWebSocketWorker {
    public:

        /// \brief Get the singleton instance of SimpleWebSocketWorker.
        /// \return Reference to the singleton instance.
        static SimpleWebSocketWorker& get_instance() {
            static SimpleWebSocketWorker instance;
            return instance;
        }

        /// \brief Provides access to the I/O context for WebSocket operations.
        /// \return Shared pointer to the I/O context.
        std::shared_ptr<SimpleWeb::io_context> get_io_context() {
            return m_io_context;
        }

        /// \brief Notifies the worker to check for pending tasks.
        void notify() {
            std::lock_guard<std::mutex> locker(m_notify_mutex);
            m_notify_condition.notify_one();
            m_notify = true;
        }

        /// \brief Starts the worker thread if it is not already running.
        void start() {
            std::unique_lock<std::mutex> locker(m_is_worker_started_mutex);
            if (m_is_worker_started) return;
            m_is_worker_started = true;
            locker.unlock();

            m_future = std::async(std::launch::async,
                    [this] {
                while (!m_shutdown) {
                    std::unique_lock<std::mutex> locker(m_notify_mutex);
                    m_notify_condition.wait(locker, [this](){
                        return m_notify;
                    });
                    m_notify = false;
                    locker.unlock();

                    try {
                        m_io_context->run();
                    } catch (...) {}

                    if (m_shutdown) break;

                    try {
                        m_io_context->restart();
                    } catch (...) {}
                }
                m_work_guard.reset();
            }).share();
        }

        /// \brief Stops the worker thread and waits for it to finish.
        void stop() {
            std::lock_guard<std::mutex> locker(m_is_worker_started_mutex);
            if (!m_future.valid()) return;
            m_shutdown = true;
            m_io_context->stop();
            notify();
            try {
                m_future.wait();
                m_future.get();
            } catch(...) {};
        }

    private:
# 		ifdef ASIO_STANDALONE
        using work_guard_t = asio::executor_work_guard<asio::io_context::executor_type>;
# 		else
        using work_guard_t = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
# 		endif

        using io_context_t = std::shared_ptr<SimpleWeb::io_context>;
        io_context_t                m_io_context;
        work_guard_t                m_work_guard;

        std::mutex                  m_is_worker_started_mutex;          ///< Mutex for controlling worker start.
        bool                        m_is_worker_started = false;        ///< Flag indicating if worker is started.

        std::mutex                  m_notify_mutex;                     ///< Mutex for notifying worker.
        std::condition_variable     m_notify_condition;                 ///< Condition variable for notifying worker.
        bool                        m_notify = false;                   ///< Flag indicating if notification is pending.

        std::shared_future<void>    m_future;
        std::atomic<bool>           m_shutdown = ATOMIC_VAR_INIT(false);

        /// \brief Private constructor to prevent instantiation.
        SimpleWebSocketWorker() :
				m_io_context(std::make_shared<SimpleWeb::io_context>()),
#           ifdef ASIO_STANDALONE
			m_work_guard(asio::make_work_guard(*m_io_context)) {
#           else
            m_work_guard(boost::asio::make_work_guard(*m_io_context)) {
#           endif
        }

        /// \brief Private destructor.
        ~SimpleWebSocketWorker() {
            stop();
        }

        // Delete copy constructor and assignment operator to enforce singleton pattern
        SimpleWebSocketWorker(const SimpleWebSocketWorker&) = delete;
        SimpleWebSocketWorker& operator=(const SimpleWebSocketWorker&) = delete;

    }; // SimpleWebSocketWorker

}; // namespace kurlyk

#endif // _KURLYK_SIMPLE_WEB_SOCKET_WORKER_HPP_INCLUDED
