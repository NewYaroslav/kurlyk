#pragma once
#ifndef _KURLYK_HTTP_RATE_LIMITER_HPP_INCLUDED
#define _KURLYK_HTTP_RATE_LIMITER_HPP_INCLUDED

/// \file HttpRateLimiter.hpp
/// \brief Defines the HttpRateLimiter class for managing rate limits on HTTP requests.

namespace kurlyk {

    /// \class HttpRateLimiter
    /// \brief Manages rate limits for HTTP requests, ensuring compliance with set limits.
    ///
    /// Each rate limit is assigned a unique identifier, allowing specific limits to be applied
    /// to different request streams. This class controls the rate of requests by monitoring the
    /// number of requests within a specified time window.
    class HttpRateLimiter {
    public:

        /// \brief Creates a new rate limit with specified parameters.
        /// This method initializes a new rate limit and returns its unique identifier.
        /// \param requests_per_period Maximum number of requests allowed within the time period. A value of 0 means no limit is applied.
        /// \param period_ms Duration of the time period in milliseconds.
        /// \return Unique identifier for the created rate limit.
        long create_limit(long requests_per_period, long period_ms) {
            std::lock_guard<std::mutex> lock(m_mutex);
            long id = m_next_id++;
            m_limits[id] = LimitData{
                requests_per_period,
                period_ms,
                0,
                std::chrono::steady_clock::now()
            };
            return id;
        }

        /// \brief Removes a rate limit with the specified ID.
        /// This method allows the removal of an existing rate limit by its unique identifier.
        /// \param limit_id The unique identifier of the rate limit to remove.
        /// \return True if the rate limit was removed successfully, false if the ID was not found.
        bool remove_limit(long limit_id) {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_limits.erase(limit_id) > 0;
        }

        /// \brief Checks if a request is allowed under the specified general and specific rate limits.
        /// This method checks both the general and specific rate limits without updating their counters
        /// unless the request is allowed under both limits. If the request is allowed, it updates the
        /// counters accordingly.
        /// \param general_rate_limit_id Unique identifier for the general rate limit.
        /// \param specific_rate_limit_id Unique identifier for the specific rate limit.
        /// \return True if the request is allowed under both limits, false otherwise.
        bool allow_request(long general_rate_limit_id, long specific_rate_limit_id) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto general_it = m_limits.find(general_rate_limit_id);
            auto specific_it = m_limits.find(specific_rate_limit_id);
            if (general_it == m_limits.end() &&
                specific_it == m_limits.end()) return true;

            auto now = std::chrono::steady_clock::now();
            bool general_limit_allowed = true;
            bool specific_limit_allowed = true;

            if (general_it != m_limits.end()) {
                auto& limit_data = general_it->second;
                general_limit_allowed = check_limit(limit_data, now);
            }

            if (specific_it != m_limits.end()) {
                auto& limit_data = specific_it->second;
                specific_limit_allowed = check_limit(limit_data, now);
            }

            if (general_limit_allowed && specific_limit_allowed) {
                // All limits allow the request, now update counts
                if (general_it != m_limits.end()) {
                    auto& limit_data = general_it->second;
                    update_limit(limit_data, now);
                }
                if (specific_it != m_limits.end()) {
                    auto& limit_data = specific_it->second;
                    update_limit(limit_data, now);
                }
                return true;
            }

            // Request is not allowed under one or more limits
            return false;
        }

        /// \brief Calculates the delay until the next request is allowed under the specified rate limits.
        ///
        /// Computes the maximum wait time between the general and specific limits.
        /// Ensures that a request is only allowed when **all** applicable limits permit it.
        ///
        /// \tparam Duration Duration type (e.g., std::chrono::milliseconds, std::chrono::microseconds).
        /// \param general_rate_limit_id ID of the general rate limit.
        /// \param specific_rate_limit_id ID of the specific rate limit.
        /// \return Duration to wait before the request is safe to perform. Zero if already allowed by both limits.
        template<typename Duration = std::chrono::milliseconds>
        Duration time_until_next_allowed(long general_rate_limit_id, long specific_rate_limit_id) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto now = std::chrono::steady_clock::now();
            Duration max_delay{0};

            auto it = m_limits.find(general_rate_limit_id);
            if (it != m_limits.end()) {
                max_delay = std::max(max_delay, time_until_limit_allows<Duration>(it->second, now));
            }

            it = m_limits.find(specific_rate_limit_id);
            if (it != m_limits.end()) {
                max_delay = std::max(max_delay, time_until_limit_allows<Duration>(it->second, now));
            }

            return max_delay;
        }

        /// \brief Finds the shortest delay among all active rate limits.
        ///
        /// Returns the minimum duration until any of the rate limits allows a request.
        /// If all limits are currently available, returns zero.
        ///
        /// \tparam Duration The chrono duration type (e.g., std::chrono::milliseconds, std::chrono::microseconds).
        /// \return The shortest wait duration among all limits. Zero if no wait is required.
        template<typename Duration = std::chrono::milliseconds>
        Duration time_until_any_limit_allows() {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto now = std::chrono::steady_clock::now();

            Duration min_delay = Duration::max();

            for (const auto& pair : m_limits) {
                const auto& limit = pair.second;
                Duration delay = time_until_limit_allows<Duration>(limit, now);
                if (delay.count() > 0 && delay < min_delay) {
                    min_delay = delay;
                }
            }

            return (min_delay == Duration::max()) ? Duration{0} : min_delay;
        }

    private:
        using time_point_t = std::chrono::steady_clock::time_point;

        /// \struct LimitData
        /// \brief Stores data for an individual rate limit.
        struct LimitData {
            long requests_per_period;   ///< Max requests allowed in the period.
            long period_ms;             ///< Duration of the period in milliseconds.
            long count;                 ///< Requests made in the current period.
            time_point_t start_time;    ///< Start time of the current rate period.
        };

        std::mutex m_mutex;             ///< Mutex to protect shared data.
        long m_next_id = 1;             ///< Next available unique ID for rate limits.
        std::unordered_map<long, LimitData> m_limits; ///< Map storing rate limit configurations.

        /// \brief Checks if a request is allowed under the specified rate limit without updating the count.
        /// \param limit_data Reference to the LimitData for the rate limit.
        /// \param now The current time point.
        /// \return True if the request is allowed, false otherwise.
        bool check_limit(LimitData& limit_data, const time_point_t& now) {
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - limit_data.start_time);

            // If period has elapsed, the count will reset upon update
            if (elapsed_time.count() >= limit_data.period_ms) {
                return true; // Period elapsed, limit can be reset
            }

            // Check if under limit or if no limit is set (0 requests per period means unlimited)
            if (limit_data.count < limit_data.requests_per_period ||
                limit_data.requests_per_period == 0) {
                return true;
            }
            return false;
        }

        /// \brief Updates the request count for the specified rate limit.
        /// \param limit_data Reference to the LimitData for the rate limit.
        /// \param now The current time point.
        void update_limit(LimitData& limit_data, const time_point_t& now) {
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - limit_data.start_time);

            // Reset count if period has elapsed
            if (elapsed_time.count() >= limit_data.period_ms) {
                limit_data.start_time = now;
                limit_data.count = 0;
            }

            ++limit_data.count;
        }

        /// \brief Calculates the delay required before a request is allowed under a single rate limit.
        ///
        /// If the request exceeds the allowed number within the configured period, returns the remaining time
        /// until the rate limit resets. Otherwise returns zero. The time unit is determined by the template argument.
        ///
        /// \tparam Duration Duration type (e.g., std::chrono::milliseconds, std::chrono::microseconds).
        /// \param limit_data Internal data for the rate limit being evaluated.
        /// \param now Current timestamp to compare against the rate period start time.
        /// \return Duration to wait until the rate limit allows a request. Zero if already allowed.
        template<typename Duration>
        Duration time_until_limit_allows(const LimitData& limit_data, const time_point_t& now) const {
            if (limit_data.requests_per_period == 0) return Duration{0};
            auto elapsed = std::chrono::duration_cast<Duration>(now - limit_data.start_time);
            auto period_duration = std::chrono::duration_cast<Duration>(std::chrono::milliseconds(limit_data.period_ms));
            if (elapsed >= period_duration ||
                limit_data.count < limit_data.requests_per_period) {
                return Duration{0};
            }
            return period_duration - elapsed;
        }
    }; // HttpRateLimiter

} // namespace kurlyk

#endif // _KURLYK_HTTP_RATE_LIMITER_HPP_INCLUDED
