#pragma once
#ifndef KURLYK_HEADER_KURLYK_HTTP_HTTP_REQUEST_MANAGER_HTTP_RATE_LIMIT_HANDLE_HPP_INCLUDED
#define KURLYK_HEADER_KURLYK_HTTP_HTTP_REQUEST_MANAGER_HTTP_RATE_LIMIT_HANDLE_HPP_INCLUDED

/// \file HttpRateLimitHandle.hpp
/// \brief Defines RAII handle for keeping HTTP rate limits alive.

namespace kurlyk {

    /// \class HttpRateLimitHandle
    /// \brief RAII handle that owns a registered HTTP rate-limit ID.
    ///
    /// This object is intended to be stored through `HttpRateLimitHandlePtr`.
    /// Each copied `std::shared_ptr` keeps the corresponding rate-limit entry alive.
    ///
    /// When the last shared handle is destroyed, the handle invokes the stored
    /// removal callback and physically removes the rate-limit entry from the
    /// owning rate limiter.
    ///
    /// \note The handle object itself is non-copyable. Copy `HttpRateLimitHandlePtr`
    ///       instead.
    class HttpRateLimitHandle {
    public:
        HttpRateLimitHandle(const HttpRateLimitHandle&) = delete;
        HttpRateLimitHandle& operator=(const HttpRateLimitHandle&) = delete;

        /// \brief Destroys the handle and releases the owned rate-limit entry.
        ~HttpRateLimitHandle() {
            reset();
        }

        /// \brief Returns immutable rate-limit ID associated with this handle.
        /// \return Rate-limit ID, or `0` if the handle has already been reset.
        long id() const {
            return m_id;
        }

        /// \brief Checks whether the handle still owns a valid rate-limit ID.
        /// \return True if the handle owns a non-zero rate-limit ID.
        explicit operator bool() const {
            return m_id != 0;
        }

    private:
        friend class HttpRateLimiter;

        using remove_fn_t = std::function<void(long)>;

        /// \brief Creates a handle for a registered rate-limit ID.
        /// \param id Registered rate-limit ID.
        /// \param remove_fn Callback used to physically remove the rate-limit entry.
        HttpRateLimitHandle(long id, remove_fn_t remove_fn)
            : m_id(id),
              m_remove_fn(std::move(remove_fn)) {}

        /// \brief Releases the owned rate-limit entry once.
        ///
        /// The ID is cleared before invoking the removal callback to prevent
        /// repeated removal if `reset()` is ever called more than once.
        void reset() {
            const long id = m_id;
            m_id = 0;

            if (id != 0 && m_remove_fn) {
                m_remove_fn(id);
            }
        }

    private:
        long        m_id = 0;    ///< Owned rate-limit ID, or `0` after reset.
        remove_fn_t m_remove_fn; ///< Callback that physically removes the rate-limit entry.
    };

    /// \brief Shared RAII handle for HTTP rate limits.
    ///
    /// Requests should store this pointer to keep the referenced rate limit alive
    /// while they are pending, active, or waiting for retry.
    using HttpRateLimitHandlePtr = std::shared_ptr<HttpRateLimitHandle>;

} // namespace kurlyk

#endif // KURLYK_HEADER_KURLYK_HTTP_HTTP_REQUEST_MANAGER_HTTP_RATE_LIMIT_HANDLE_HPP_INCLUDED
