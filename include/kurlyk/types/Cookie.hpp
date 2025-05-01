#pragma once
#ifndef _KURLYK_TYPES_COOKIE_HPP_INCLUDED
#define _KURLYK_TYPES_COOKIE_HPP_INCLUDED

/// \file Cookie.hpp
/// \brief Defines the Cookie class for managing HTTP cookies.

namespace kurlyk {

    /// \class Cookie
    /// \brief Represents an HTTP cookie.
    class Cookie {
    public:
        std::string name;               ///< The name of the cookie.
        std::string value;              ///< The value of the cookie.
        std::string path;               ///< The path to which the cookie is associated.
        uint64_t expiration_date = 0;   ///< The expiration date of the cookie in Unix time format.

        /// \brief Default constructor for the Cookie class.
        Cookie() = default;

        /// \brief Parameterized constructor for the Cookie class.
        /// \param name The name of the cookie.
        /// \param value The value of the cookie.
        /// \param path The path to which the cookie is associated.
        /// \param expiration_date The expiration date of the cookie in Unix time format.
        Cookie(const std::string& name, const std::string& value, const std::string& path = std::string(), uint64_t expiration_date = 0)
            : name(name), value(value), path(path), expiration_date(expiration_date) {}
    };

}; // namespace kurlyk

#endif // _KURLYK_TYPES_COOKIE_HPP_INCLUDED
