#pragma once
#ifndef _KURLYK_HTTP_REQUEST_MANAGER_HPP_INCLUDED
#define _KURLYK_HTTP_REQUEST_MANAGER_HPP_INCLUDED

/// \file HttpRequestManager.hpp
/// \brief Manages and processes HTTP requests using a singleton pattern.

#include "HttpRequestManager/HttpRequestContext.hpp"
#include "HttpRequestManager/HttpRequestHandler.hpp"
#include "HttpRequestManager/HttpRateLimiter.hpp"
#include "HttpRequestManager/HttpBatchRequestHandler.hpp"
#include "HttpRequestManager/HttpRateLimiter.hpp"

namespace kurlyk {

    /// \class HttpRequestManager
    /// \brief Manages and processes HTTP requests using a singleton pattern.
    class HttpRequestManager final : public core::INetworkTaskManager {
    public:

        /// \brief Get the singleton instance of HttpRequestManager.
        /// \return Reference to the singleton instance.
        static HttpRequestManager& get_instance() {
            static HttpRequestManager* instance = new HttpRequestManager();
            return *instance;
        }

        /// \brief Adds a new HTTP request to the manager.
        /// \param request_ptr Unique pointer to the HTTP request object containing request details.
        /// \param callback Callback function invoked when the request completes.
        /// \return True if the request was successfully added, false if the manager is shutting down.
        const bool add_request(
                std::unique_ptr<HttpRequest> request_ptr,
                HttpResponseCallback callback) {
            if (m_shutdown) return false;
            std::lock_guard<std::mutex> lock(m_mutex);
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
        const long create_rate_limit(long requests_per_period, long period_ms) {
            return m_rate_limiter.create_limit(requests_per_period, period_ms);
        }

        /// \brief Removes an existing rate limit with the specified identifier.
        /// \param limit_id The unique identifier of the rate limit to be removed.
        /// \return True if the rate limit was successfully removed, or false if the rate limit ID was not found.
        bool remove_limit(long limit_id) {
            return m_rate_limiter.remove_limit(limit_id);
        }

        /// \brief Generates a new unique request ID.
        /// \return A new unique request ID.
        uint64_t generate_request_id() {
            return m_request_id_counter++;
        }

        /// \brief Cancels a request by its unique identifier.
        /// \param request_id The unique identifier of the request to cancel.
        /// \param callback An optional callback function to execute after cancellation.
        void cancel_request_by_id(uint64_t request_id, std::function<void()> callback) {
            if (m_shutdown) {
                if (callback) callback();
                return;
            }
            std::lock_guard<std::mutex> lock(m_mutex);
            m_requests_to_cancel[request_id].push_back(std::move(callback));
        }

        /// \brief Processes all requests in the manager.
        ///
        /// Executes pending, active, and retry-eligible failed requests.
        void process() override {
            process_pending_requests();
            process_active_requests();
            process_retry_failed_requests();
            process_cancel_requests();
        }

        /// \brief Shuts down the request manager, clearing all active and pending requests.
        /// Stops request processing and releases all resources tied to active and pending requests.
        void shutdown() override {
            m_shutdown = true;
            cleanup_pending_requests();
            process_cancel_requests();
            m_active_request_batches.clear();
        }

        /// \brief Checks if there are active, pending, or failed requests.
        /// \return True if there are requests still being managed, otherwise false.
        const bool is_loaded() const override {
            std::lock_guard<std::mutex> lock(m_mutex);
            return
                !m_pending_requests.empty() ||
                !m_failed_requests.empty() ||
                !m_active_request_batches.empty() ||
                !m_requests_to_cancel.empty();
        }

    private:
        mutable std::mutex                                  m_mutex;                  ///< Mutex to protect access to the pending requests list and requests-to-cancel map.
        std::list<std::unique_ptr<HttpRequestContext>>      m_pending_requests;       ///< List of pending HTTP requests awaiting processing.
        std::list<std::unique_ptr<HttpRequestContext>>      m_failed_requests;        ///< List of failed HTTP requests for retrying.
        std::list<std::unique_ptr<HttpBatchRequestHandler>> m_active_request_batches; ///< List of currently active HTTP request batches.
        using callback_list_t = std::list<std::function<void()>>;
        std::unordered_map<uint64_t, callback_list_t>       m_requests_to_cancel;     ///< Map of request IDs to their associated cancellation callbacks.
        HttpRateLimiter                                     m_rate_limiter;           ///< Rate limiter for controlling request frequency.
        std::atomic<uint64_t>                               m_request_id_counter = ATOMIC_VAR_INIT(1); ///< Atomic counter for unique request IDs.
        std::atomic<bool>                                   m_shutdown = ATOMIC_VAR_INIT(false); ///< Flag indicating if shutdown has been requested.

        /// \brief Processes all pending requests, moving valid requests to active batches or marking them as failed.
        void process_pending_requests() {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_pending_requests.empty()) return;

            std::vector<std::unique_ptr<HttpRequestContext>> pending_request;
            std::vector<std::unique_ptr<HttpRequestContext>> failed_requests;

            auto it = m_pending_requests.begin();
            while (it != m_pending_requests.end()) {
                auto& context = *it;
                auto& request = context->request;
                // Check if the request is valid.
                if (!request) {
                    failed_requests.push_back(std::move(context));
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
                pending_request.push_back(std::move(context));
                it = m_pending_requests.erase(it);
            }
            lock.unlock();

            // Handle failed requests by calling their callback with a 400 status.
            if (!failed_requests.empty()) {
                for (const auto &context : failed_requests) {
#                   if __cplusplus >= 201402L
                    auto response = std::make_unique<HttpResponse>();
#                   else
                    auto response = std::unique_ptr<HttpResponse>(new HttpResponse());
#                   endif
                    const long BAD_REQUEST = 400;
                    response->error_code = utils::make_error_code(CURLE_OK);
                    response->status_code = BAD_REQUEST;
                    response->ready = true;
                    context->callback(std::move(response));
                }
                failed_requests.clear();
            }

            // If there are ready requests, create a new HttpBatchRequestHandler to manage them.
            if (pending_request.empty()) return;
#           if __cplusplus >= 201402L
            m_active_request_batches.push_back(std::make_unique<HttpBatchRequestHandler>(pending_request));
#           else
            m_active_request_batches.push_back(std::unique_ptr<HttpBatchRequestHandler>(new HttpBatchRequestHandler(pending_request)));
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
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_pending_requests.push_back(std::move(request_context));
                    lock.unlock();
                    it = m_failed_requests.erase(it);
                    continue;
                }
                ++it;
            }
        }

        /// \brief Processes and cancels HTTP requests based on their IDs.
        void process_cancel_requests() {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_requests_to_cancel.empty()) return;
            auto requests_to_cancel = std::move(m_requests_to_cancel);
            m_requests_to_cancel.clear();
            lock.unlock();

            for (const auto &request_context : m_failed_requests) {
                if (!requests_to_cancel.count(request_context->request->request_id)) continue;
#               if __cplusplus >= 201402L
                auto response = std::make_unique<HttpResponse>();
#               else
                auto response = std::unique_ptr<HttpResponse>(new HttpResponse());
#               endif
                const long CANCELED_REQUEST_CODE = 499;
                response->error_code = utils::make_error_code(CURLE_OK);
                response->status_code = CANCELED_REQUEST_CODE;
                response->ready = true;
                request_context->callback(std::move(response));
            }

            for (const auto &handler : m_active_request_batches) {
                handler->cancel_request_by_id(requests_to_cancel);
            }

            for (const auto &request : requests_to_cancel) {
                for (const auto &callback : request.second) {
                    if (callback) callback();
                }
            }
        }

        /// \brief Cleans up pending requests, marking each as failed and invoking its callback.
        void cleanup_pending_requests() {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto pending_requests = std::move(m_pending_requests);
            m_pending_requests.clear();
            lock.unlock();

            for (const auto &request_context : pending_requests) {
#               if __cplusplus >= 201402L
                auto response = std::make_unique<HttpResponse>();
#               else
                auto response = std::unique_ptr<HttpResponse>(new HttpResponse());
#               endif
                const long CANCELED_REQUEST_CODE = 499;
                response->error_code = utils::make_error_code(CURLE_OK);
                response->status_code = CANCELED_REQUEST_CODE;
                response->ready = true;
                request_context->callback(std::move(response));
            }
            for (const auto &request_context : m_failed_requests) {
#               if __cplusplus >= 201402L
                auto response = std::make_unique<HttpResponse>();
#               else
                auto response = std::unique_ptr<HttpResponse>(new HttpResponse());
#               endif
                const long CANCELED_REQUEST_CODE = 499;
                response->error_code = utils::make_error_code(CURLE_OK);
                response->status_code = CANCELED_REQUEST_CODE;
                response->ready = true;
                request_context->callback(std::move(response));
            }
        }

        /// \brief Private constructor to initialize global resources (e.g., cURL).
        HttpRequestManager() {
            curl_global_init(CURL_GLOBAL_ALL);
        }

        /// \brief Private destructor to clean up global resources.
        virtual ~HttpRequestManager() {
            curl_global_cleanup();
        }

        /// \brief Deleted copy constructor to enforce the singleton pattern.
        HttpRequestManager(const HttpRequestManager&) = delete;

        /// \brief Deleted copy assignment operator to enforce the singleton pattern.
        HttpRequestManager& operator=(const HttpRequestManager&) = delete;

    }; // HttpRequestManager

}; // namespace kurlyk

#endif // _KURLYK_HTTP_REQUEST_MANAGER_HPP_INCLUDED
