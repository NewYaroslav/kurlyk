#pragma once
#ifndef _KURLYK_HTTP_RATE_LIMIT_LEASE_HPP_INCLUDED
#define _KURLYK_HTTP_RATE_LIMIT_LEASE_HPP_INCLUDED

/// \file HttpRateLimitLease.hpp
/// \brief Defines RAII lease for releasing serialized HTTP rate-limit ownership.

namespace kurlyk {

    /// \class HttpRateLimitLease
    /// \brief RAII lease that owns temporary serialized rate-limit locks.
    ///
    /// This object is intended to be stored through `HttpRateLimitLeasePtr`.
    /// A request context can keep one lease while it owns serialized rate-limit
    /// resources, such as general/specific rate limits or cooldown keys.
    ///
    /// When the last shared lease is destroyed, the lease invokes the stored
    /// release callback and releases all serialized resources owned by this
    /// request context.
    ///
    /// \note The lease object itself is non-copyable. Copy `HttpRateLimitLeasePtr`
    ///       instead.
    class HttpRateLimitLease {
    public:
        /// \struct LimitLock
        /// \brief Describes serialized ownership of one rate-limit ID.
        struct LimitLock {
            long     limit_id = 0; ///< Serialized rate-limit ID.
            uint64_t queue_id = 0; ///< Queue owner ID that currently owns this limit.
        };

        /// \struct CooldownLock
        /// \brief Describes serialized ownership of one cooldown key.
        struct CooldownLock {
            std::string key;       ///< Cooldown key owned by this lease.
            uint64_t    queue_id = 0; ///< Queue owner ID that currently owns this key.
        };

        HttpRateLimitLease(const HttpRateLimitLease&) = delete;
        HttpRateLimitLease& operator=(const HttpRateLimitLease&) = delete;

        /// \brief Destroys the lease and releases all owned serialized resources.
        ~HttpRateLimitLease() {
            reset();
        }

        /// \brief Returns queue owner ID associated with this lease.
        /// \return Queue owner ID, or `0` if the lease has already been reset.
        uint64_t queue_id() const {
            return m_queue_id;
        }

        /// \brief Checks whether the lease still owns any serialized resources.
        /// \return True if the lease has not been reset and owns at least one resource.
        explicit operator bool() const {
            return m_active && (!m_limit_locks.empty() || !m_cooldown_locks.empty());
        }

    private:
        friend class HttpRateLimiter;

        using release_fn_t = std::function<void(
            uint64_t,
            const std::vector<LimitLock>&,
            const std::vector<CooldownLock>&)>;

        /// \brief Creates a lease for serialized rate-limit resources.
        /// \param queue_id Queue owner ID associated with this lease.
        /// \param limit_locks Serialized rate-limit locks owned by this lease.
        /// \param cooldown_locks Serialized cooldown-key locks owned by this lease.
        /// \param release_fn Callback used to release owned serialized resources.
        HttpRateLimitLease(
                uint64_t queue_id,
                std::vector<LimitLock> limit_locks,
                std::vector<CooldownLock> cooldown_locks,
                release_fn_t release_fn)
            : m_queue_id(queue_id),
              m_limit_locks(std::move(limit_locks)),
              m_cooldown_locks(std::move(cooldown_locks)),
              m_release_fn(std::move(release_fn)),
              m_active(true) {}

        /// \brief Releases all owned serialized resources once.
        ///
        /// The lease is marked inactive before invoking the release callback to
        /// prevent repeated release if `reset()` is ever called more than once.
        void reset() {
            if (!m_active) {
                return;
            }

            m_active = false;

            const uint64_t queue_id = m_queue_id;
            m_queue_id = 0;

            if (m_release_fn &&
                (!m_limit_locks.empty() || !m_cooldown_locks.empty())) {
                m_release_fn(queue_id, m_limit_locks, m_cooldown_locks);
            }

            m_limit_locks.clear();
            m_cooldown_locks.clear();
        }

    private:
        uint64_t                  m_queue_id = 0;       ///< Queue owner ID, or `0` after reset.
        std::vector<LimitLock>    m_limit_locks;        ///< Serialized rate-limit locks owned by this lease.
        std::vector<CooldownLock> m_cooldown_locks;     ///< Serialized cooldown-key locks owned by this lease.
        release_fn_t              m_release_fn;         ///< Callback that releases owned serialized resources.
        bool                      m_active = false;     ///< True while the lease has not been reset.
    };

    /// \brief Shared RAII lease for serialized HTTP rate-limit ownership.
    ///
    /// Request contexts should store this pointer while they own serialized
    /// rate-limit resources. The lease should normally live across retry
    /// attempts and be released only when the request context is finally
    /// completed, cancelled, or destroyed.
    using HttpRateLimitLeasePtr = std::shared_ptr<HttpRateLimitLease>;

} // namespace kurlyk

#endif // _KURLYK_HTTP_RATE_LIMIT_LEASE_HPP_INCLUDED