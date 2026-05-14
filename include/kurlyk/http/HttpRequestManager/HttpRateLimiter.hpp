#pragma once
#ifndef _KURLYK_HTTP_RATE_LIMITER_HPP_INCLUDED
#define _KURLYK_HTTP_RATE_LIMITER_HPP_INCLUDED

/// \file HttpRateLimiter.hpp
/// \brief Defines the HttpRateLimiter class for managing rate limits on HTTP requests.

#include <cstdint>
#include <unordered_set>

namespace kurlyk {

    /// \class HttpRateLimiter
    /// \brief Manages rate limits for HTTP requests.
    ///
    /// Rate limits are referenced by RAII handles. A limit remains physically
    /// present while at least one HttpRateLimitHandlePtr exists. Calling
    /// remove_limit(id) only releases the manager-owned handle. If pending or
    /// active requests still hold copied handles, the limit remains active until
    /// those requests are destroyed.
    class HttpRateLimiter {
    public:
        /// \brief Creates a new rate limit and returns its RAII handle.
        /// \param requests_per_period Maximum requests allowed within the period. 0 means unlimited.
        /// \param period_ms Period duration in milliseconds.
        /// \param sequential When true, no other request sharing this limit may start until
        ///        the current request (including all its retries) has finished.
        /// \return Shared RAII handle for the created limit.
        HttpRateLimitHandlePtr create_limit_handle(long requests_per_period, long period_ms, bool sequential = false) {
            std::lock_guard<std::mutex> lock(m_mutex);

            const long id = m_next_id++;

            m_limits[id] = LimitData{
                requests_per_period,
                period_ms,
                0,
                std::chrono::steady_clock::now(),
                sequential,
                false,
                std::unordered_set<uint64_t>()
            };

            // Do not use make_shared here: private constructor access through
            // std::make_shared can be problematic on some compilers.
            HttpRateLimitHandlePtr handle(
                new HttpRateLimitHandle(
                    id,
                    [this](long limit_id) {
                        remove_limit_internal(limit_id);
                    }
                )
            );

            m_owned_handles[id] = handle;
            return handle;
        }

        /// \brief Legacy API: creates a new rate limit and returns only its ID.
        ///
        /// \warning Prefer create_limit_handle(). Requests that only store the ID
        /// do not keep the limit alive by themselves. The manager-owned handle
        /// keeps the limit alive until remove_limit(id) is called.
        long create_limit(long requests_per_period, long period_ms) {
            const auto handle = create_limit_handle(requests_per_period, period_ms);
            return handle ? handle->id() : 0;
        }

        /// \brief Releases manager-owned handle for the specified limit ID.
        ///
        /// This does not necessarily erase LimitData immediately. If any pending
        /// or active request still holds a copied handle, the limit stays alive.
        /// Physical erase happens from HttpRateLimitHandle destructor.
        ///
        /// \param limit_id Rate-limit ID.
        /// \return true if manager-owned handle was found and released.
        bool remove_limit(long limit_id) {
            HttpRateLimitHandlePtr retired_handle;

            std::unique_lock<std::mutex> lock(m_mutex);

            auto it = m_owned_handles.find(limit_id);
            if (it == m_owned_handles.end()) {
                return false;
            }

            retired_handle = std::move(it->second);
            m_owned_handles.erase(it);

            lock.unlock();

            // retired_handle is destroyed after m_mutex is released.
            // If it is the last shared_ptr, its destructor calls
            // remove_limit_internal(limit_id).
            return true;
        }

        /// \brief Releases manager-owned handle for the specified limit handle.
        /// \param handle Rate-limit handle.
        /// \return true if manager-owned handle was found and released.
        bool remove_limit(const HttpRateLimitHandlePtr& handle) {
            return handle ? remove_limit(handle->id()) : false;
        }

        /// \brief Returns manager-owned handle by ID.
        ///
        /// \note This is mainly for compatibility with legacy ID-based API.
        HttpRateLimitHandlePtr get_limit(long limit_id) {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto it = m_owned_handles.find(limit_id);
            if (it == m_owned_handles.end()) {
                return HttpRateLimitHandlePtr();
            }

            return it->second;
        }

        /// \brief Checks if a request is allowed by two optional rate-limit handles
        ///        with in-flight token tracking for sequential limits.
        ///
        /// Counters and in-flight sets are updated only if both limits allow the request.
        /// \param general_limit General rate-limit handle (may be empty).
        /// \param specific_limit Specific rate-limit handle (may be empty).
        /// \param in_flight_token Token identifying the in-flight request; 0 skips sequential checks.
        /// \return True if the request is allowed, false otherwise (state unchanged on failure).
        bool allow_request(
                const HttpRateLimitHandlePtr& general_limit,
                const HttpRateLimitHandlePtr& specific_limit,
                uint64_t in_flight_token
            ) {
            std::lock_guard<std::mutex> lock(m_mutex);

            const long general_id = general_limit ? general_limit->id() : 0;
            const long specific_id = specific_limit ? specific_limit->id() : 0;

            auto general_it = general_id != 0 ? m_limits.find(general_id) : m_limits.end();
            auto specific_it = specific_id != 0 ? m_limits.find(specific_id) : m_limits.end();

            if (general_it == m_limits.end() && specific_it == m_limits.end()) {
                return true;
            }

            const auto now = std::chrono::steady_clock::now();

            bool general_limit_allowed = true;
            bool specific_limit_allowed = true;

            if (general_it != m_limits.end()) {
                if (general_it->second.removed) {
                    general_limit_allowed = true;
                } else {
                    general_limit_allowed = can_pass(general_it->second, in_flight_token, now);
                }
            }

            if (specific_it != m_limits.end()) {
                if (specific_it->second.removed) {
                    specific_limit_allowed = true;
                } else {
                    specific_limit_allowed = can_pass(specific_it->second, in_flight_token, now);
                }
            }

            if (!general_limit_allowed || !specific_limit_allowed) {
                return false;
            }

            const bool same_limit =
                general_id != 0 &&
                general_id == specific_id;

            if (general_it != m_limits.end() && !general_it->second.removed) {
                commit_limit(general_it->second, in_flight_token, now);
            }

            if (!same_limit && specific_it != m_limits.end() && !specific_it->second.removed) {
                commit_limit(specific_it->second, in_flight_token, now);
            }

            return true;
        }

        /// \brief Handle-based overload without explicit token (token = 0, skips sequential checks).
        bool allow_request(
                const HttpRateLimitHandlePtr& general_limit,
                const HttpRateLimitHandlePtr& specific_limit
            ) {
            return allow_request(general_limit, specific_limit, 0);
        }

        /// \brief Legacy API: checks if request is allowed by two limit IDs.
        ///
        /// \warning Prefer handle-based overload. ID-based requests do not keep
        /// limits alive by themselves.
        bool allow_request(long general_rate_limit_id, long specific_rate_limit_id) {
            return allow_request(
                get_limit(general_rate_limit_id),
                get_limit(specific_rate_limit_id),
                0
            );
        }

        /// \brief Releases in-flight tokens for sequential rate limits.
        ///
        /// Must be called when a request finishes (including after cancellation or destruction).
        /// \param general_limit General rate-limit handle (may be empty).
        /// \param specific_limit Specific rate-limit handle (may be empty).
        /// \param in_flight_token Token identifying the in-flight request; 0 is a no-op.
        void release_request(
                const HttpRateLimitHandlePtr& general_limit,
                const HttpRateLimitHandlePtr& specific_limit,
                uint64_t in_flight_token) {
            if (in_flight_token == 0) return;

            std::lock_guard<std::mutex> lock(m_mutex);

            const long general_id = general_limit ? general_limit->id() : 0;
            const long specific_id = specific_limit ? specific_limit->id() : 0;

            auto general_it = general_id != 0 ? m_limits.find(general_id) : m_limits.end();
            auto specific_it = specific_id != 0 ? m_limits.find(specific_id) : m_limits.end();

            if (general_it != m_limits.end() && general_it->second.sequential) {
                general_it->second.in_flight_tokens.erase(in_flight_token);
                if (general_it->second.removed && general_it->second.in_flight_tokens.empty()) {
                    m_limits.erase(general_it);
                }
            }

            if (specific_it != m_limits.end() && specific_it->second.sequential) {
                specific_it->second.in_flight_tokens.erase(in_flight_token);
                if (specific_it->second.removed && specific_it->second.in_flight_tokens.empty()) {
                    m_limits.erase(specific_it);
                }
            }
        }

        /// \brief Calculates delay until request is allowed by two handles.
        template<typename Duration = std::chrono::milliseconds>
        Duration time_until_next_allowed(
            const HttpRateLimitHandlePtr& general_limit,
            const HttpRateLimitHandlePtr& specific_limit
            ) {
            std::lock_guard<std::mutex> lock(m_mutex);

            const auto now = std::chrono::steady_clock::now();
            Duration max_delay{0};

            const long general_id = general_limit ? general_limit->id() : 0;
            const long specific_id = specific_limit ? specific_limit->id() : 0;

            auto it = general_id != 0 ? m_limits.find(general_id) : m_limits.end();
            if (it != m_limits.end()) {
                max_delay = std::max(
                    max_delay,
                    time_until_limit_allows<Duration>(it->second, now)
                );
            }

            it = specific_id != 0 ? m_limits.find(specific_id) : m_limits.end();
            if (it != m_limits.end()) {
                max_delay = std::max(
                    max_delay,
                    time_until_limit_allows<Duration>(it->second, now)
                );
            }

            return max_delay;
        }

        /// \brief Legacy API: calculates delay by limit IDs.
        template<typename Duration = std::chrono::milliseconds>
        Duration time_until_next_allowed(long general_rate_limit_id, long specific_rate_limit_id) {
            return time_until_next_allowed<Duration>(
                get_limit(general_rate_limit_id),
                get_limit(specific_rate_limit_id)
            );
        }

        /// \brief Finds the shortest delay among all physically alive limits.
        template<typename Duration = std::chrono::milliseconds>
        Duration time_until_any_limit_allows() {
            std::lock_guard<std::mutex> lock(m_mutex);

            const auto now = std::chrono::steady_clock::now();
            Duration min_delay = Duration::max();
            bool has_blocking_delay = false;

            for (const auto& pair : m_limits) {
                const auto& limit = pair.second;

                const Duration delay = time_until_limit_allows<Duration>(limit, now);
                if (delay.count() <= 0) {
                    continue;
                }

                has_blocking_delay = true;

                if (delay < min_delay) {
                    min_delay = delay;
                }
            }

            return has_blocking_delay ? min_delay : Duration{0};
        }

    private:
        using time_point_t = std::chrono::steady_clock::time_point;

        /// \struct LimitData
        /// \brief Internal state for one rate limit.
        struct LimitData {
            long requests_per_period = 0;
            long period_ms = 0;
            long count = 0;
            time_point_t start_time;
            bool sequential = false;                               ///< When true, blocks other requests until the current one finishes.
            bool removed = false;                                   ///< True when the manager-owned handle has been released; physical erase is deferred until in_flight_tokens is empty.
            std::unordered_set<uint64_t> in_flight_tokens;        ///< Tokens of requests currently owning this sequential limit.
        };

        /// \brief Physically removes LimitData from m_limits.
        ///
        /// This method is intentionally private. It must be called only from
        /// HttpRateLimitHandle destruction path.
        /// \note HttpRequestManager (and its embedded HttpRateLimiter) is intentionally
        /// never destroyed. The `this` captured in HttpRateLimitHandle destructor is
        /// therefore always valid. Changing the singleton to a destructible form requires
        /// replacing this raw capture.
        bool remove_limit_internal(long limit_id) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_limits.find(limit_id);
            if (it == m_limits.end()) {
                return false;
            }
            auto& limit = it->second;
            limit.removed = true;
            if (!limit.in_flight_tokens.empty()) {
                return false;
            }
            return m_limits.erase(limit_id) > 0;
        }

        bool check_limit(const LimitData& limit_data, const time_point_t& now) const {
            if (limit_data.requests_per_period == 0) {
                return true;
            }

            const auto elapsed_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - limit_data.start_time
                );

            if (elapsed_time.count() >= limit_data.period_ms) {
                return true;
            }

            return limit_data.count < limit_data.requests_per_period;
        }

        /// \brief Checks if a request can pass both sequential and count/period constraints.
        ///
        /// Sequential logic:
        /// - If !limit.sequential || token == 0 -> ignore sequential.
        /// - Else if limit.in_flight_tokens.empty() -> pass sequential.
        /// - Else if limit.in_flight_tokens.count(token) -> pass sequential (our retry).
        /// - Else -> fail sequential.
        /// Then check existing count/period logic.
        bool can_pass(const LimitData& limit_data, uint64_t token, const time_point_t& now) const {
            if (limit_data.sequential && token != 0) {
                if (!limit_data.in_flight_tokens.empty() &&
                    limit_data.in_flight_tokens.count(token) == 0) {
                    return false;
                }
            }
            return check_limit(limit_data, now);
        }

        /// \brief Commits in-flight token and count/period state after a successful can_pass.
        void commit_limit(LimitData& limit_data, uint64_t token, const time_point_t& now) {
            if (limit_data.sequential && token != 0) {
                limit_data.in_flight_tokens.insert(token);
            }
            update_limit(limit_data, now);
        }

        void update_limit(LimitData& limit_data, const time_point_t& now) {
            if (limit_data.requests_per_period == 0) {
                return;
            }

            const auto elapsed_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - limit_data.start_time
                );

            if (elapsed_time.count() >= limit_data.period_ms) {
                limit_data.start_time = now;
                limit_data.count = 0;
            }

            ++limit_data.count;
        }

        template<typename Duration>
        Duration time_until_limit_allows(
            const LimitData& limit_data,
            const time_point_t& now
        ) const {
            if (limit_data.sequential &&
                !limit_data.in_flight_tokens.empty()) {
                return Duration::max();
            }

            if (limit_data.requests_per_period == 0) {
                return Duration{0};
            }

            const auto elapsed =
                std::chrono::duration_cast<Duration>(now - limit_data.start_time);

            const auto period_duration =
                std::chrono::duration_cast<Duration>(
                    std::chrono::milliseconds(limit_data.period_ms)
                );

            if (elapsed >= period_duration ||
                limit_data.count < limit_data.requests_per_period) {
                return Duration{0};
            }

            return period_duration - elapsed;
        }

    private:
        mutable std::mutex m_mutex;

        long m_next_id = 1;

        /// \brief Physically alive limit data.
        ///
        /// A record remains here while at least one HttpRateLimitHandlePtr exists.
        std::unordered_map<long, LimitData> m_limits;

        /// \brief Manager-owned handles for limits created through create_limit_handle().
        ///
        /// remove_limit(id) releases the handle from this map. If requests still
        /// hold copies, physical data stays alive until the last copy is destroyed.
        std::unordered_map<long, HttpRateLimitHandlePtr> m_owned_handles;
    };

} // namespace kurlyk

#endif // _KURLYK_HTTP_RATE_LIMITER_HPP_INCLUDED
