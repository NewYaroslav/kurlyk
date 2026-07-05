#pragma once
#ifndef KURLYK_HEADER_KURLYK_UTILS_PKCE_HPP_INCLUDED
#define KURLYK_HEADER_KURLYK_UTILS_PKCE_HPP_INCLUDED

/// \file Pkce.hpp
/// \brief Provides PKCE (Proof Key for Code Exchange) utilities per RFC 7636.

#include "Base64Url.hpp"
#include <hmac_cpp/sha256.hpp>
#include <hmac_cpp/hmac_utils.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace kurlyk {
namespace utils {

    /// \class PkcePair
    /// \brief Stores PKCE verifier and challenge values.
    struct PkcePair {
        std::string code_verifier;          ///< Randomly generated code verifier.
        std::string code_challenge;         ///< Derived S256 code challenge.
        std::string code_challenge_method = "S256"; ///< Challenge method, always "S256".
    };

    /// \brief Generates a cryptographically strong PKCE code verifier.
    /// \param length Verifier length, must be between 43 and 128 (inclusive).
    /// \return Base64url-encoded random string of the requested length.
    inline std::string generate_code_verifier(std::size_t length = 64) {
        static const char allowed[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
        if (length < 43) length = 43;
        if (length > 128) length = 128;

        std::vector<uint8_t> random_bytes = hmac_cpp::random_bytes(length);
        std::string verifier;
        verifier.reserve(length);

        const std::size_t allowed_count = sizeof(allowed) - 1; // exclude null terminator
        for (std::size_t i = 0; i < length; ++i) {
            verifier.push_back(allowed[random_bytes[i] % allowed_count]);
        }
        return verifier;
    }

    /// \brief Creates an S256 code challenge from a verifier.
    /// \param verifier PKCE code verifier.
    /// \return Base64url-encoded SHA256 hash without padding.
    inline std::string make_s256_code_challenge(const std::string& verifier) {
        std::vector<uint8_t> digest = hmac_hash::sha256(
            reinterpret_cast<const uint8_t*>(verifier.data()),
            verifier.size());
        return base64url_encode(digest.data(), digest.size());
    }

    /// \brief Creates a PKCE pair with a freshly generated verifier.
    /// \return Generated PKCE pair.
    inline PkcePair make_pkce_pair() {
        PkcePair pair;
        pair.code_verifier = generate_code_verifier();
        pair.code_challenge = make_s256_code_challenge(pair.code_verifier);
        return pair;
    }

} // namespace utils
} // namespace kurlyk

#endif // KURLYK_HEADER_KURLYK_UTILS_PKCE_HPP_INCLUDED
