#pragma once
#ifndef _KURLYK_HTTP_MULTI_REQUEST_HANDLER_HPP_INCLUDED
#define _KURLYK_HTTP_MULTI_REQUEST_HANDLER_HPP_INCLUDED

/// \file MultiRequestHandler.hpp
/// \brief Manages multiple asynchronous HTTP requests using libcurl's multi interface.

#include "RequestHandler.hpp"
#include <list>
#include <unordered_map>
#include <memory>

namespace kurlyk {

    /// \class HttpMultiRequestHandler
    /// \brief Handles multiple asynchronous HTTP requests using libcurl's multi interface.
    class HttpMultiRequestHandler {
    public:

        /// \brief Constructs a handler for managing multiple HTTP requests asynchronously.
        /// \param request_list List of unique pointers to HttpRequestContext objects.
        explicit HttpMultiRequestHandler(std::list<std::unique_ptr<HttpRequestContext>>& request_list)
            : m_multi_handle(curl_multi_init()) {
            for (auto& request : request_list) {
#               if __cplusplus >= 201402L
                auto request_handler = std::make_unique<HttpRequestHandler>(std::move(request));
#               else
                auto request_handler = std::unique_ptr<HttpRequestHandler>(new HttpRequestHandler(std::move(request)));
#               endif
                CURL* curl = request_handler->get_curl();
                if (!curl) continue;

                curl_multi_add_handle(m_multi_handle, curl);
                m_curl_to_handler_map[curl] = std::move(request_handler);
            }
        }

        /// \brief Cleans up the multi handle and removes all request handles.
        ~HttpMultiRequestHandler() {
            for (auto& entry : m_curl_to_handler_map) {
                curl_multi_remove_handle(m_multi_handle, entry.first);
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
                if (message->msg == CURLMSG_DONE) {
                    handle_completed_request(message);
                }
            }
            return (still_running == 0);
        }

        /// \brief Extracts the list of failed requests.
        /// \return A list of failed request contexts.
        std::list<std::unique_ptr<HttpRequestContext>> extract_failed_requests() {
            return std::move(m_failed_requests);
        }

        /// \brief Cancels HTTP requests based on their unique IDs.
        /// \param requests_to_cancel A map of request IDs to their corresponding cancellation callbacks.
        void cancel_request_by_id(const std::unordered_map<uint64_t, std::list<std::function<void()>>>& requests_to_cancel) {
            auto it = m_curl_to_handler_map.begin();
            while (it != m_curl_to_handler_map.end()) {
                if (!requests_to_cancel.count(it->second->get_request_id())) {
                    it++;
                    continue;
                }
                it->second->cancel(); // Cancel the request.
                curl_multi_remove_handle(m_multi_handle, it->first);
                it = m_curl_to_handler_map.erase(it);
            }
        }

    private:
        CURLM* m_multi_handle = nullptr; ///< libcurl multi handle.
        std::unordered_map<CURL*, std::unique_ptr<HttpRequestHandler>> m_curl_to_handler_map; ///< Map from CURL handles to request handlers.
        std::list<std::unique_ptr<HttpRequestContext>> m_failed_requests; ///< List of failed request contexts.

        /// \brief Handles the completion of a single request.
        /// \param message CURLMsg structure containing the result of the completed request.
        void handle_completed_request(CURLMsg* message) {
            CURL* curl = message->easy_handle;
            auto it = m_curl_to_handler_map.find(curl);
            if (it == m_curl_to_handler_map.end()) return;

            if (!it->second->handle_curl_message(message)) {
                m_failed_requests.push_back(it->second->get_request_context());
            }

            curl_multi_remove_handle(m_multi_handle, curl);
            m_curl_to_handler_map.erase(curl);
        }

    }; // HttpMultiRequestHandler

} // namespace kurlyk

#endif // _KURLYK_HTTP_MULTI_REQUEST_HANDLER_HPP_INCLUDED
