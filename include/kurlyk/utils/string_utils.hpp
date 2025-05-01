#pragma once
#ifndef _KURLYK_UTILS_STRING_UTILS_HPP_INCLUDED
#define _KURLYK_UTILS_STRING_UTILS_HPP_INCLUDED

/// \file string_utils.hpp
/// \brief Provides basic string manipulation utilities.

#include <string>
#include <algorithm>

namespace kurlyk::utils {

    /// \brief Converts a string to uppercase.
    /// \param str Input string.
    /// \return A new string converted to uppercase.
    inline std::string to_upper_case(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });
        return str;
    }

    /// \brief Converts a string to lowercase.
    /// \param str Input string.
    /// \return A new string converted to lowercase.
    inline std::string to_lower_case(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return str;
    }

} // namespace kurlyk::utils

#endif // _KURLYK_UTILS_STRING_UTILS_HPP_INCLUDED
