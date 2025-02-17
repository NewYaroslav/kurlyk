#pragma once
#ifndef _KURLYK_NETWORK_WORKER_HPP_INCLUDED
#define _KURLYK_NETWORK_WORKER_HPP_INCLUDED

/// \file NetworkWorker.hpp
/// \brief Manages and processes network tasks such as HTTP requests and WebSocket events in a separate thread.

#if KURLYK_HTTP_SUPPORT
#include "Http/RequestManager.hpp"
#endif

#if KURLYK_WEBSOCKET_SUPPORT
#include "WebSocket/Manager.hpp"
#endif

#include <future>

namespace kurlyk {

    /// \class NetworkWorker
    /// \brief Singleton worker that manages asynchronous network operations, including HTTP requests and WebSocket events.
    ///
    /// The `NetworkWorker` is responsible for managing and processing network-related tasks in a separate thread.
    /// It supports adding tasks to a queue, notifying and starting the worker thread, and handling graceful shutdown.
    class NetworkWorker {
    public:

        /// \brief Get the singleton instance of NetworkWorker.
        /// \return Reference to the singleton instance.
        static NetworkWorker& get_instance() {
            static NetworkWorker* instance = new NetworkWorker();
            return *instance;
        }

        /// \brief Adds a task to the queue and notifies the worker thread.
        /// \param task A function or lambda with no arguments to be executed by the worker.
        void add_task(std::function<void()> task) {
            std::unique_lock<std::mutex> lock(m_tasks_list_mutex);
            m_tasks_list.push_back(std::move(task));
            lock.unlock();
            notify();
        }

        /// \brief Processes all queued tasks and active HTTP and WebSocket requests.
        ///
        /// Processes pending tasks in the task list and manages network requests in both HTTP and WebSocket managers.
        void process() {
#           if KURLYK_HTTP_SUPPORT
            HttpRequestManager::get_instance().process();
#           endif
#           if KURLYK_WEBSOCKET_SUPPORT
            WebSocketManager::get_instance().process();
#           endif
            process_tasks();
        }

        /// \brief Notifies the worker to begin processing requests or tasks.
        ///
        /// Signals the condition variable to wake up the worker thread if it is waiting, allowing tasks to be processed.
        void notify() {
            std::lock_guard<std::mutex> locker(m_notify_mutex);
            m_notify_condition.notify_one();
            m_notify = true;
        }

        /// \brief Starts the worker thread for asynchronous task processing.
        ///
        /// If `use_async` is true, the worker runs in a separate thread, continually processing tasks and network events
        /// until `stop()` is called.
        /// \param use_async Indicates whether the worker should run asynchronously.
        void start(const bool use_async) {
            std::unique_lock<std::mutex> locker(m_is_worker_started_mutex);
            if (m_is_worker_started) return;
            m_is_worker_started = true;
            if (!use_async) return;
            locker.unlock();

            m_future = std::async(std::launch::async,
                    [this] {
                for (;;) {
                    std::unique_lock<std::mutex> locker(m_notify_mutex);
                    m_notify_condition.wait(locker, [this](){
                        return m_notify;
                    });
                    m_notify = false;
                    locker.unlock();

                    if (m_shutdown) {
                        shutdown();
                        return;
                    }

                    while (is_loaded()) {
                        process();
                        if (m_shutdown) {
                            shutdown();
                            return;
                        }

                        std::unique_lock<std::mutex> locker(m_notify_mutex);
                        m_notify_condition.wait_for(locker, std::chrono::milliseconds(1), [this] {
                            return m_notify || m_shutdown;
                        });
                        m_notify = false;
                        locker.unlock();

                        if (m_shutdown) {
                            shutdown();
                            return;
                        }
                    }

                    if (m_shutdown) {
                        shutdown();
                        return;
                    }
                }
            }).share();
        }

        /// \brief Stops the worker thread, ensuring all tasks are completed.
        ///
        /// Signals the worker thread to stop and waits for it to complete all tasks before fully shutting down.
        void stop() {
            if (!m_future.valid()) return;
            m_shutdown = true;
            notify();
            try {
                m_future.wait();
                m_future.get();
            } catch(...) {};
        }

        /// \brief Shuts down the worker, clearing all active requests and pending tasks.
        ///
        /// Stops both HTTP and WebSocket managers and processes any remaining tasks in the queue.
        void shutdown() {
#           if KURLYK_HTTP_SUPPORT
            HttpRequestManager::get_instance().shutdown();
#           endif
#           if KURLYK_WEBSOCKET_SUPPORT
            WebSocketManager::get_instance().shutdown();
#           endif
            process_tasks();
        }

    private:
        std::shared_future<void>    m_future;                           ///< Future for managing asynchronous worker execution.
        std::atomic<bool>           m_shutdown = ATOMIC_VAR_INIT(false);///< Flag indicating if shutdown has been requested.
        std::mutex                  m_notify_mutex;                     ///< Mutex for managing worker notifications.
        std::condition_variable     m_notify_condition;                 ///< Condition variable for notifying the worker.
        bool                        m_notify = false;                   ///< Flag indicating whether a notification is pending.
        std::mutex                  m_is_worker_started_mutex;          ///< Mutex to control worker thread initialization.
        bool                        m_is_worker_started = false;        ///< Flag indicating if the worker thread is started.
        mutable std::mutex          m_tasks_list_mutex;                 ///< Mutex for protecting access to the task list.
        std::list<std::function<void()>> m_tasks_list;                  ///< List of tasks queued for processing by the worker.

        /// \brief Private constructor to enforce singleton pattern.
        NetworkWorker() {
            utils::convert_user_agent_to_sec_ch_ua("");
            utils::is_valid_email_id("");
        }

        /// \brief Private destructor, ensuring the worker thread stops on destruction.
        ~NetworkWorker() {
            stop();
        }

        /// \brief Deleted copy constructor to enforce the singleton pattern.
        NetworkWorker(const NetworkWorker&) = delete;

        /// \brief Deleted copy assignment operator to enforce the singleton pattern.
        NetworkWorker& operator=(const NetworkWorker&) = delete;

        /// \brief Processes all tasks in the task list, then clears the list.
        ///
        /// This method moves tasks from the task list to a local list, processes each one, and then clears the local list.
        void process_tasks() {
            std::unique_lock<std::mutex> lock(m_tasks_list_mutex);
            if (m_tasks_list.empty()) return;
            auto tasks_list = std::move(m_tasks_list);
            m_tasks_list.clear();
            lock.unlock();
            for (auto &item : tasks_list) {
                item();
            }
            tasks_list.clear();
        }

        /// \brief Checks if there are any pending tasks in the task list.
        /// \return True if there are pending tasks, otherwise false.
        const bool has_pending_tasks() const {
            std::lock_guard<std::mutex> lock(m_tasks_list_mutex);
            return !m_tasks_list.empty();
        }

        /// \brief Checks if the NetworkWorker has pending tasks or active network events.
        ///
        /// Checks if there are any tasks or events in the HTTP or WebSocket manager or in the task list.
        /// \return True if there are tasks or events to process; otherwise, false.
        const bool is_loaded() const {
#           if KURLYK_HTTP_SUPPORT == 1 && KURLYK_WEBSOCKET_SUPPORT == 1
            return HttpRequestManager::get_instance().is_loaded() ||
                WebSocketManager::get_instance().is_loaded() || has_pending_tasks();
#           elif KURLYK_HTTP_SUPPORT == 1 && KURLYK_WEBSOCKET_SUPPORT == 0
            return RequestManager::get_instance().is_loaded() || has_pending_tasks();
#           elif KURLYK_HTTP_SUPPORT == 0 && KURLYK_WEBSOCKET_SUPPORT == 1
            return WebSocketManager::get_instance().is_loaded() || has_pending_tasks();
#           else
            return false;
#           endif
        }

    }; // NetworkWorker

}; // namespace kurlyk

#endif // _KURLYK_NETWORK_WORKER_HPP_INCLUDED
