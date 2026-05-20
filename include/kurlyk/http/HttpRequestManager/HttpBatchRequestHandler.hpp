#pragma once
#ifndef _KURLYK_HTTP_MULTI_REQUEST_HANDLER_HPP_INCLUDED
#define _KURLYK_HTTP_MULTI_REQUEST_HANDLER_HPP_INCLUDED

/// \file HttpBatchRequestHandler.hpp
/// \brief Manages multiple asynchronous HTTP requests using libcurl's multi interface.

namespace kurlyk {

    /// \class HttpBatchRequestHandler
    /// \brief Handles multiple asynchronous HTTP requests using libcurl's multi interface.
    class HttpBatchRequestHandler {
    public:

        /// \brief Constructs a handler for managing multiple HTTP requests asynchronously.
        /// \param context_list List of unique pointers to HttpRequestContext objects.
        explicit HttpBatchRequestHandler(std::vector<std::unique_ptr<HttpRequestContext>>& context_list)
            : m_multi_handle(curl_multi_init()) {
            for (auto& context : context_list) {
#               if __cplusplus >= 201402L
                auto handler = std::make_unique<HttpRequestHandler>(std::move(context));
#               else
                auto handler = std::unique_ptr<HttpRequestHandler>(new HttpRequestHandler(std::move(context)));
#               endif
                CURL* curl = handler->get_curl();
                if (!curl) continue;

                curl_multi_add_handle(m_multi_handle, curl);
                m_handlers.push_back(std::move(handler));
            }
        }

        /// \brief Cleans up the multi handle and removes all request handles.
        ~HttpBatchRequestHandler() {
            for (auto& handler : m_handlers) {
                CURL* curl = handler->get_curl();
                if (!curl) continue;
                curl_multi_remove_handle(m_multi_handle, curl);
            }
            curl_multi_cleanup(m_multi_handle);
        }

        /// \brief Processes the requests within the handler.
        /// \return True if all requests are completed, false otherwise.
        bool process() {
            int still_running = 0;
            CURLMcode res = curl_multi_perform(m_multi_handle, &still_running);
            if (res != CURLM_OK) return false;

            int pending_messages;
            while (CURLMsg* message = curl_multi_info_read(m_multi_handle, &pending_messages)) {
                if (message->msg != CURLMSG_DONE) continue;
                handle_completed_request(message);
            }
            if (still_running == 0) {
                m_handlers.clear();
                return true;
            }
            return false;
        }

        /// \brief Extracts the list of failed requests.
        /// \return A list of failed request contexts.
        std::list<std::unique_ptr<HttpRequestContext>> extract_failed_requests() {
            return std::move(m_failed_requests);
        }

        /// \brief Checks whether this batch contains a request from the specified group.
        /// \param group_id Group ID to inspect.
        /// \return True if at least one active request belongs to this group.
        bool has_group_id(uint64_t group_id) const {
            return group_request_count(group_id) != 0;
        }

        /// \brief Counts active requests from the specified group.
        /// \param group_id Group ID to inspect.
        /// \return Number of active requests belonging to this group.
        std::size_t group_request_count(uint64_t group_id) const {
            if (group_id == 0) return 0;

            std::size_t count = 0;
            for (const auto& handler : m_handlers) {
                if (handler && handler->get_group_id() == group_id) {
                    ++count;
                }
            }
            return count;
        }

        /// \brief Cancels HTTP requests based on their unique IDs.
        /// \param to_cancel A map of request IDs to their corresponding cancellation callbacks.
        void cancel_request_by_id(const std::unordered_map<uint64_t, std::list<std::function<void()>>>& to_cancel) {
            cancel_requests(to_cancel, std::unordered_map<uint64_t, std::list<std::function<void()>>>());
        }

        /// \brief Cancels HTTP requests based on their request or group IDs.
        /// \param requests_to_cancel A map of request IDs to their corresponding cancellation callbacks.
        /// \param groups_to_cancel A map of group IDs to their corresponding cancellation callbacks.
        void cancel_requests(
                const std::unordered_map<uint64_t, std::list<std::function<void()>>>& requests_to_cancel,
                const std::unordered_map<uint64_t, std::list<std::function<void()>>>& groups_to_cancel) {
            auto it = m_handlers.begin();
            while (it != m_handlers.end()) {
                const uint64_t request_id = (*it)->get_request_id();
                const uint64_t group_id = (*it)->get_group_id();
                const bool should_cancel =
                    (request_id != 0 && requests_to_cancel.count(request_id) > 0) ||
                    (group_id != 0 && groups_to_cancel.count(group_id) > 0);
                if (!should_cancel) {
                    ++it;
                    continue;
                }
                curl_multi_remove_handle(m_multi_handle, (*it)->get_curl());
                (*it)->cancel();
                it = m_handlers.erase(it);
            }
        }

    private:
        CURLM* m_multi_handle = nullptr; ///< libcurl multi handle.
        std::vector<std::unique_ptr<HttpRequestHandler>> m_handlers; ///< Collection of active request handlers.
        std::list<std::unique_ptr<HttpRequestContext>>   m_failed_requests; ///< List of failed request contexts.

        /// \brief Handles the completion of a single request.
        /// \param message CURLMsg structure containing the result of the completed request.
        void handle_completed_request(CURLMsg* message) {
            CURL* curl = message->easy_handle;

            void* ptr = nullptr;
            curl_easy_getinfo(curl, CURLINFO_PRIVATE, &ptr);
            auto* handler = static_cast<HttpRequestHandler*>(ptr);
            if (!handler) return;

            if (!handler->handle_curl_message(message)) {
                m_failed_requests.push_back(handler->get_request_context());
            }
            curl_multi_remove_handle(m_multi_handle, curl);
        }

    }; // HttpBatchRequestHandler

} // namespace kurlyk

#endif // _KURLYK_HTTP_MULTI_REQUEST_HANDLER_HPP_INCLUDED
