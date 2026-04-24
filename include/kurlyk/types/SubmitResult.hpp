#pragma once
#ifndef _KURLYK_TYPES_SUBMIT_RESULT_HPP_INCLUDED
#define _KURLYK_TYPES_SUBMIT_RESULT_HPP_INCLUDED

/// \file SubmitResult.hpp
/// \brief Defines the SubmitResult structure for synchronous admission results.

#include <system_error>

namespace kurlyk {

    /// \struct SubmitResult
    /// \brief Represents the synchronous result of trying to enqueue or submit work.
    struct SubmitResult {
        bool            accepted = false;   ///< Indicates whether the work item was accepted for processing.
        std::error_code error_code;         ///< Describes the rejection reason when `accepted` is false.

        /// \brief Converts the result to a boolean accepted flag.
        explicit operator bool() const {
            return accepted;
        }
    };

} // namespace kurlyk

#endif // _KURLYK_TYPES_SUBMIT_RESULT_HPP_INCLUDED
