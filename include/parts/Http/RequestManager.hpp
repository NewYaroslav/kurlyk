#pragma once
#ifndef _KURLYK_HTTP_REQUEST_MANAGER_HPP_INCLUDED
#define _KURLYK_HTTP_REQUEST_MANAGER_HPP_INCLUDED

/// \file HttpRequestManager.hpp
/// \brief Manages and processes HTTP requests using a singleton pattern.

#include "RequestManager/MultiRequestHandler.hpp"
#include "RequestManager/RateLimiter.hpp"
#include <atomic>

namespace kurlyk {

    /// \class HttpRequestManager
    /// \brief Manages and processes HTTP requests using a singleton pattern.
    class HttpRequestManager {
    public:

        /// \brief Get the singleton instance of HttpRequestManager.
        /// \return Reference to the singleton instance.
        static HttpRequestManager& get_instance() {
            static HttpRequestManager instance;
            return instance;
        }

        /// \brief Adds a new HTTP request to the manager.
        /// \param request_ptr Unique pointer to the HTTP request object containing request details.
        /// \param callback Callback function invoked when the request completes.
        /// \return True if the request was successfully added, false if the manager is shutting down.
        const bool add_request(
                std::unique_ptr<HttpRequest> request_ptr,
                HttpResponseCallback callback) {
            if (m_shutdown) return false;
            std::lock_guard<std::mutex> lock(m_pending_requests_mutex);
#           if __cplusplus >= 201402L
            m_pending_requests.push_back(std::make_unique<HttpRequestContext>(std::move(request_ptr), std::move(callback)));
#           else
            m_pending_requests.push_back(std::unique_ptr<HttpRequestContext>(
                new HttpRequestContext(std::move(request_ptr), std::move(callback))));
#           endif
            return true;
        }

        /// \brief Creates a rate limit with specified parameters.
        /// \param requests_per_period Maximum number of requests allowed in the specified period.
        /// \param period_ms Time period in milliseconds during which the rate limit applies.
        /// \return A unique identifier for the created rate limit.
        const long create_rate_limit(const long& requests_per_period, const long& period_ms) {
            return m_rate_limiter.create_limit(requests_per_period, period_ms);
        }

        /// \brief Processes all requests in the manager.
        ///
        /// Executes pending, active, and retry-eligible failed requests.
        void process() {
            process_pending_requests();
            process_active_requests();
            process_retry_failed_requests();
        }

        /// \brief Shuts down the request manager, clearing all active and pending requests.
        ///
        /// Stops request processing and releases all resources tied to active and pending requests.
        void shutdown() {
            m_shutdown = true;
            cleanup_pending_requests();
            m_active_request_batches.clear();
        }

        /// \brief Checks if there are active, pending, or failed requests.
        /// \return True if there are requests still being managed, otherwise false.
        const bool is_loaded() const {
            std::lock_guard<std::mutex> lock(m_pending_requests_mutex);
            return
                !m_pending_requests.empty() ||
                !m_failed_requests.empty() ||
                !m_active_request_batches.empty();
        }

    private:
        mutable std::mutex                                  m_pending_requests_mutex; ///< Mutex to protect access to the pending requests list.
        std::list<std::unique_ptr<HttpRequestContext>>      m_pending_requests;       ///< List of pending HTTP requests awaiting processing.
        std::list<std::unique_ptr<HttpRequestContext>>      m_failed_requests;        ///< List of failed HTTP requests for retrying.
        std::list<std::unique_ptr<HttpMultiRequestHandler>> m_active_request_batches; ///< List of currently active HTTP request batches.
        HttpRateLimiter                                     m_rate_limiter;           ///< Rate limiter for controlling request frequency.
        std::atomic<bool>                                   m_shutdown = ATOMIC_VAR_INIT(false); ///< Flag indicating if shutdown has been requested.

        /// \brief Processes all pending requests, moving valid requests to active batches or marking them as failed.
        void process_pending_requests() {
            std::unique_lock<std::mutex> lock(m_pending_requests_mutex);
            if (m_pending_requests.empty()) return;

            std::list<std::unique_ptr<HttpRequestContext>> pending_request;
            std::list<std::unique_ptr<HttpRequestContext>> failed_requests;

            auto it = m_pending_requests.begin();
            while (it != m_pending_requests.end()) {
                auto& request_context = *it;
                auto& request = request_context->request;
                // Check if the request is valid.
                if (!request) {
                    failed_requests.push_back(std::move(request_context));
                    it = m_pending_requests.erase(it);
                    continue;
                }

                // Check if the request is allowed by the rate limiter.
                const bool allowed = m_rate_limiter.allow_request(
                    request->general_rate_limit_id,
                    request->specific_rate_limit_id);
                if (!allowed) {
                    ++it;
                    continue;
                }
                pending_request.push_back(std::move(request_context));
                it = m_pending_requests.erase(it);
            }
            lock.unlock();

            // Handle failed requests by calling their callback with a 400 status.
            if (!failed_requests.empty()) {
                for (const auto &request_context : failed_requests) {
#                   if __cplusplus >= 201402L
                    auto response = std::make_unique<HttpResponse>();
#                   else
                    auto response = std::unique_ptr<HttpResponse>(new HttpResponse());
#                   endif
                    const long BAD_REQUEST = 400;
                    response->error_code = utils::make_error_code(CURLE_OK);
                    response->status_code = BAD_REQUEST;
                    response->ready = true;
                    request_context->callback(std::move(response));
                }
                failed_requests.clear();
            }

            // If there are ready requests, create a new HttpMultiRequestHandler to manage them.
            if (pending_request.empty()) return;
#           if __cplusplus >= 201402L
            m_active_request_batches.push_back(std::make_unique<HttpMultiRequestHandler>(pending_request));
#           else
            m_active_request_batches.push_back(std::unique_ptr<HttpMultiRequestHandler>(new HttpMultiRequestHandler(pending_request)));
#           endif
        }

        /// \brief Processes active requests, moving failed ones to the failed requests list for retrying.
        void process_active_requests() {
            auto it = m_active_request_batches.begin();
            while (it != m_active_request_batches.end()) {
                auto& request = *it;
                if (!request->process()) {
                    ++it;
                    continue;
                }
                auto failed_requests = request->extract_failed_requests();
                it = m_active_request_batches.erase(it);
                for (auto& request : failed_requests) {
                    m_failed_requests.push_back(std::move(request));
                }
            }
        }

         /// \brief Attempts to retry failed requests if their retry delay has passed.
        void process_retry_failed_requests() {
            auto it = m_failed_requests.begin();
            while (it != m_failed_requests.end()) {
                auto& request_context = *it;
                if (!request_context || !request_context->request) {
                    it = m_failed_requests.erase(it);
                    continue;
                }

                const auto now = std::chrono::steady_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - request_context->start_time);
                const auto& retry_delay_ms = request_context->request->retry_delay_ms;
                if (duration.count() >= retry_delay_ms) {
                    std::unique_lock<std::mutex> lock(m_pending_requests_mutex);
                    m_pending_requests.push_back(std::move(request_context));
                    lock.unlock();
                    it = m_failed_requests.erase(it);
                    continue;
                }
                ++it;
            }
        }

        /// \brief Cleans up pending requests, marking each as failed and invoking its callback.
        void cleanup_pending_requests() {
            std::unique_lock<std::mutex> lock(m_pending_requests_mutex);
            auto pending_requests = std::move(m_pending_requests);
            m_pending_requests.clear();
            lock.unlock();

            for (const auto &request_context : pending_requests) {
#               if __cplusplus >= 201402L
                auto response = std::make_unique<HttpResponse>();
#               else
                auto response = std::unique_ptr<HttpResponse>(new HttpResponse());
#               endif
                const long BAD_REQUEST = 400;
                response->error_code = utils::make_error_code(CURLE_OK);
                response->status_code = BAD_REQUEST;
                response->ready = true;
                request_context->callback(std::move(response));
            }
            for (const auto &request_context : m_failed_requests) {
#               if __cplusplus >= 201402L
                auto response = std::make_unique<HttpResponse>();
#               else
                auto response = std::unique_ptr<HttpResponse>(new HttpResponse());
#               endif
                const long BAD_REQUEST = 400;
                response->error_code = utils::make_error_code(CURLE_OK);
                response->status_code = BAD_REQUEST;
                response->ready = true;
                request_context->callback(std::move(response));
            }
        }

        /// \brief Private constructor to initialize global resources (e.g., cURL).
        HttpRequestManager() {
            curl_global_init(CURL_GLOBAL_ALL);
        }

        /// \brief Private destructor to clean up global resources.
        ~HttpRequestManager() {
            curl_global_cleanup();
        }

        /// \brief Deleted copy constructor to enforce the singleton pattern.
        HttpRequestManager(const HttpRequestManager&) = delete;

        /// \brief Deleted copy assignment operator to enforce the singleton pattern.
        HttpRequestManager& operator=(const HttpRequestManager&) = delete;

    }; // HttpRequestManager

}; // namespace kurlyk

#endif // _KURLYK_HTTP_REQUEST_MANAGER_HPP_INCLUDED
