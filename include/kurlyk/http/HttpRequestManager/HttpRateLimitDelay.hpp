#pragma once
#ifndef _KURLYK_HTTP_RATE_LIMIT_DELAY_HPP_INCLUDED
#define _KURLYK_HTTP_RATE_LIMIT_DELAY_HPP_INCLUDED

/// \file HttpRateLimitDelay.hpp
/// \brief Defines the RateLimitDelay result type for time-until-allowed queries. 

namespace kurlyk {

/// \struct RateLimitDelay
/// \brief Result type for time-until-allowed queries.
///
/// Carries both a numeric delay and a semantic flag that distinguishes
/// time-based blocking (count/period limits) from sequential blocking
/// (an in-flight request must finish before the next one may start).
///
/// When `sequential_blocked == true`, `duration` equals `Duration::max()`.
/// Callers must wait for `release_request()` or an explicit notification
/// instead of calling `sleep_for(duration)`.
template<typename Duration = std::chrono::milliseconds>
struct RateLimitDelay {
    Duration duration;       ///< Delay until the limit allows a request. 0 means ready now.
    bool sequential_blocked; ///< true if `duration` reflects `Duration::max()` because a sequential in-flight request is blocking.
                           ///< Callers must treat this as "wait for release_request(), not sleep_for()".
};

} // namespace kurlyk

#endif // _KURLYK_HTTP_RATE_LIMIT_DELAY_HPP_INCLUDED
