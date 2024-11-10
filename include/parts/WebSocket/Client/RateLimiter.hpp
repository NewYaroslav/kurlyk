#pragma once
#ifndef _KURLYK_WEB_SOCKET_RATE_LIMITER_HPP_INCLUDED
#define _KURLYK_WEB_SOCKET_RATE_LIMITER_HPP_INCLUDED

/// \file RateLimiter.hpp
/// \brief Defines WebSocket rate limiting to control the frequency of WebSocket requests.

#include <mutex>
#include <chrono>
#include "../Components/Config.hpp"

namespace kurlyk {

    /// \class WebSocketRateLimiter
    /// \brief Manages rate limiting for WebSocket requests based on predefined limits.
    /// The limiter applies a general rate limit (id 0) to all requests by default.
    class WebSocketRateLimiter {
    public:
        using time_point_t = std::chrono::steady_clock::time_point;

        /// \brief Sets the rate limits to control WebSocket request frequency.
        /// \param rate_limits A vector of rate limit configurations for different request categories.
        void set_limit(const std::vector<WebSocketConfig::RateLimitData>& rate_limits) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_limit_data.resize(rate_limits.size());
            for (size_t i = 0; i < rate_limits.size(); ++i) {
                m_limit_data[i].requests_per_period = rate_limits[i].requests_per_period;
                m_limit_data[i].period_ms = rate_limits[i].period_ms;
                m_limit_data[i].start_time = std::chrono::steady_clock::now();
            }
        }

        /// \brief Checks if a request is allowed under the specified rate limit.
        /// \param rate_limit_id The ID of the rate limit category to apply.
        /// \return true if the request is allowed, false if the request exceeds the limit.
        /*
        bool allow_request(long rate_limit_id) {
            if (rate_limit_id < 0) return true;  // No rate limit if ID is negative
            std::lock_guard<std::mutex> lock(m_mutex);
            if (rate_limit_id >= static_cast<long>(m_limit_data.size())) return true; // Out of bounds, allow by default
            // Apply both the general limit (id 0) and the specific limit if applicable
            return allow_request_impl(0) && (rate_limit_id == 0 || allow_request_impl(rate_limit_id));
        }
        */
        bool allow_request(long rate_limit_id) {
            if (rate_limit_id < 0) return true;  // No rate limit if ID is negative
            std::lock_guard<std::mutex> lock(m_mutex);
            if (rate_limit_id >= static_cast<long>(m_limit_data.size())) return true; // Out of bounds, allow by default

            // First check all applicable limits without updating counts
            bool general_limit_allowed = check_limit(0);
            bool specific_limit_allowed = (rate_limit_id == 0) || check_limit(rate_limit_id);

            if (general_limit_allowed && specific_limit_allowed) {
                // All limits allow the request, now update counts
                update_limit(0);
                if (rate_limit_id != 0) {
                    update_limit(rate_limit_id);
                }
                return true;
            }

            // Request is not allowed under one or more limits
            return false;
        }

    private:

        /// \struct LimitData
        struct LimitData {
            long requests_per_period = 0; ///< Maximum requests allowed per period.
            long period_ms = 0;           ///< Duration of the rate limit period in milliseconds.
            long count = 0;               ///< Number of requests made in the current period.
            time_point_t start_time = std::chrono::steady_clock::now(); ///< Start time of the current rate limit period.

            LimitData() = default;
        };

        std::mutex m_mutex;                     ///< Mutex to protect shared data.
        std::vector<LimitData> m_limit_data;    ///< Vector storing rate limit configurations.

        /// \brief Checks if a request is allowed under the specified rate limit without updating the count.
        /// \param rate_limit_id The ID of the rate limit to apply.
        /// \return true if the request is allowed, false if the request exceeds the limit.
        bool check_limit(long rate_limit_id) {
            LimitData& limit_data = m_limit_data[rate_limit_id];
            auto now = std::chrono::steady_clock::now();
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

        /// \brief Updates the request count for the specified rate limit ID.
        /// \param rate_limit_id The ID of the rate limit to apply.
        void update_limit(long rate_limit_id) {
            LimitData& limit_data = m_limit_data[rate_limit_id];
            auto now = std::chrono::steady_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - limit_data.start_time);

            // Reset count if period has elapsed
            if (elapsed_time.count() >= limit_data.period_ms) {
                limit_data.start_time = now;
                limit_data.count = 0;
            }

            ++limit_data.count;
        }
    };

} // namespace kurlyk

#endif // _KURLYK_WEB_SOCKET_RATE_LIMITER_HPP_INCLUDED
