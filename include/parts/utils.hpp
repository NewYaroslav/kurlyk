#pragma once
#ifndef _KURLYK_UTILIS_HPP_INCLUDED
#define _KURLYK_UTILIS_HPP_INCLUDED

/// \file Utils.hpp
/// \brief Contains utility functions and classes for handling HTTP requests and responses.

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>

#ifdef KURLYK_USE_CURL
#include <curl/curl.h>
#include "Utils/CurlErrorCategory.hpp"
#endif

#include "Utils/EventQueue.hpp"
#include "Utils/CaseInsensitiveMultimap.hpp"

#ifdef _WIN32
// For Windows systems
#include <direct.h>
#include <windows.h>
#include <locale>
#include <codecvt>
#else
// For POSIX systems
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#endif


#define KURLYK_PRINT kurlyk::utils::ThreadSafePrintStream{}

namespace kurlyk {
namespace utils {

#   ifndef KURLYK_USE_CURL

    /// \brief Encodes a string using Percent Encoding according to RFC 3986.
    /// \param value The string to be encoded.
    /// \return The percent-encoded string.
    std::string percent_encode(const std::string &value) noexcept {
        static const char hex_chars[] = "0123456789ABCDEF";

        std::string result;
        result.reserve(value.size()); // Reserve minimum required size

        for (auto &chr : value) {
            if (isalnum(static_cast<unsigned char>(chr)) || chr == '-' || chr == '.' || chr == '_' || chr == '~') {
                result += chr;
            } else {
                result += '%';
                result += hex_chars[static_cast<unsigned char>(chr) >> 4];
                result += hex_chars[static_cast<unsigned char>(chr) & 0x0F];
            }
        }

        return result;
    }

    /// \brief Decodes a Percent-Encoded string.
    /// \param value The percent-encoded string to be decoded.
    /// \return The decoded string.
    std::string percent_decode(const std::string &value) noexcept {
        std::string result;
        result.reserve(value.size() / 3 + (value.size() % 3)); // Reserve minimum required size

        for (std::size_t i = 0; i < value.size(); ++i) {
            if (value[i] == '%' && i + 2 < value.size()) {
                char hex[] = { value[i + 1], value[i + 2], '\0' };
                char decoded_chr = static_cast<char>(std::strtol(hex, nullptr, 16));
                result += decoded_chr;
                i += 2;
            } else if (value[i] == '+') {
                result += ' ';
            } else {
                result += value[i];
            }
        }

        return result;
    }

#   endif

    /// \brief Parses a header pair from a buffer.
    /// \param buffer The buffer containing the header.
    /// \param size The size of the buffer.
    /// \param key Output parameter for the header key.
    /// \param value Output parameter for the header value.
    inline void parse_http_header_pair(
            const char *buffer,
            const size_t& size,
            std::string& key,
            std::string& value) {
        std::string header(buffer, size);
        if (header.size() < 3) return;
        std::size_t colon_pos = header.find_first_of(":");
        std::size_t start_pos = colon_pos + 1;
        if (colon_pos == std::string::npos || colon_pos == 0) return;
        key = header.substr(0, colon_pos);
        std::size_t end_pos = header.find_first_not_of(" ", start_pos);
        value = (end_pos != std::string::npos) ? header.substr(end_pos) : std::string();
    }

#   ifdef KURLYK_USE_CURL

    /// \brief Converts a map of query parameters into a URL query string.
    /// \param query The multimap containing query fields and values.
    /// \param prefix Optional prefix for the query string.
    /// \return The encoded query string.
    std::string to_query_string(
            const QueryParams &query,
            const std::string &prefix = std::string()) noexcept {
        if (query.empty()) return std::string();
        std::string query_string(prefix);
        CURL *curl = curl_easy_init();
        if (!curl) return query_string;

        size_t index = 0;
        const size_t index_end = query.size() - 1;
        for(const auto& pair : query) {
            char *escaped_key = curl_easy_escape(curl, pair.first.c_str(), pair.first.size());
            if (escaped_key) {
                query_string += std::string(escaped_key);
                curl_free(escaped_key);
            }
            query_string += "=";
            char *escaped_value = curl_easy_escape(curl, pair.second.c_str(), pair.second.size());
            if(escaped_value) {
                query_string += std::string(escaped_value);
                curl_free(escaped_value);
            }
            if (index != index_end) query_string += "&";
            ++index;
        }
        curl_easy_cleanup(curl);
        return query_string;
    }

#   else

    /// \brief Converts a map of query parameters into a URL query string.
    /// \param query The multimap containing query fields and values.
    /// \param prefix Optional prefix for the query string.
    /// \return The encoded query string.
    std::string to_query_string(
            const QueryParams &query,
            const std::string &prefix = std::string()) noexcept {
        std::string result(prefix);
        bool first = true;

        for (const auto &field : query) {
            if (!first) {
                result += '&';
            }
            result += field.first + '=' + percent_encode(field.second);
            first = false;
        }

        return result;
    }

#   endif

    /// \brief Extracts the protocol from a URL.
    /// \param url The URL string.
    /// \return The protocol scheme as a string. Returns an empty string if no protocol is found.
    inline std::string extract_protocol(const std::string& url) {
        std::string protocol;
        std::size_t pos = url.find("://");

        if (pos != std::string::npos) {
            protocol = url.substr(0, pos);
        }

        return protocol;
    }

    /// \brief Converts a CaseInsensitiveMultimap to a string format suitable for HTTP Cookie headers.
    /// \param cookies The multimap containing key-value pairs.
    /// \return A string formatted as a Cookie header.
    std::string to_cookie_string(const CaseInsensitiveMultimap& cookies) {
        std::string result;

        for (auto it = cookies.begin(); it != cookies.end(); ++it) {
            if (it != cookies.begin()) {
                result += "; ";
            }
            result += it->first + "=" + it->second;
        }

        return result;
    }

    /// \brief Converts a CaseInsensitiveMultimap to a string format suitable for HTTP Cookie headers.
    /// \param cookies The multimap containing key-value pairs.
    /// \return A string formatted as a Cookie header.
    std::string to_cookie_string(const Cookies& cookies) {
        std::string result;

        for (auto it = cookies.begin(); it != cookies.end(); ++it) {
            if (it != cookies.begin()) {
                result += "; ";
            }
            result += it->second.name + "=" + it->second.value;

            if (!it->second.path.empty()) {
                result += "; Path=" + it->second.path;
            }

            if (it->second.expiration_date != 0) {
                result += "; Expires=" + std::to_string(it->second.expiration_date);
            }
        }

        return result;
    }

    /// \brief Parses a cookie string into a Cookies object.
    /// \param cookie The cookie string to parse.
    /// \return A Cookies object containing parsed cookies.
    Cookies parse_cookie(std::string cookie) {
        Cookies cookies;
        std::vector<std::string> list_fragment;
        cookie += ";";
        const std::string secure_prefix("__Secure-");
        const std::string host_prefix("__Host-");
        std::size_t start_pos = 0;

        for (;;) {
            bool is_option = false;
            std::size_t separator_pos = cookie.find_first_of("=;", start_pos);
            if (separator_pos != std::string::npos) {
                std::string name = cookie.substr(start_pos, (separator_pos - start_pos));

                if (name.size() > host_prefix.size() && name.substr(0, 2) == "__") {
                    std::size_t prefix_pos = name.find_first_of("-");
                    if(prefix_pos != std::string::npos) {
                        name = name.substr(prefix_pos + 1, name.size() - prefix_pos - 1);
                    }
                } else {
                    if (case_insensitive_equal(name, "expires") ||
                        case_insensitive_equal(name, "max-age") ||
                        case_insensitive_equal(name, "path") ||
                        case_insensitive_equal(name, "domain") ||
                        case_insensitive_equal(name, "samesite")) {
                        is_option = true;
                    } else if (case_insensitive_equal(name, "secure") ||
                               case_insensitive_equal(name, "httponly")) {
                        start_pos = (cookie[separator_pos] == ';') ? separator_pos + 2 : separator_pos + 1;
                        continue;
                    }
                }

                std::size_t end_pos = cookie.find("; ", separator_pos + 1);
                if (end_pos == std::string::npos) {
                    std::string value = cookie.substr(separator_pos + 1, cookie.size() - separator_pos - 2);
                    Cookie cookie_obj{name, value};
                    if (!is_option) cookies.emplace(cookie_obj.name, cookie_obj);
                    break;
                }
                std::string value = cookie.substr(separator_pos + 1, end_pos - separator_pos - 1);
                start_pos = end_pos + 2;
                Cookie cookie_obj{name, value};
                if (!is_option) cookies.emplace(cookie_obj.name, cookie_obj);
            } else {
                break;
            }
        }
        return cookies;
    };

    /// \brief Removes the first occurrence of "wss://" or "ws://" from the given URL.
    /// \param url The URL from which to remove the substring.
    /// \return std::string The modified URL with the first occurrence of "wss://" or "ws://" removed.
    std::string remove_ws_prefix(const std::string& url) {
        const std::string wss_prefix = "wss://";
        const std::string ws_prefix = "ws://";

        std::string modified_url = url;

        // Find the position of "wss://" or "ws://"
        std::size_t pos = modified_url.find(wss_prefix);
        if (pos == std::string::npos) {
            pos = modified_url.find(ws_prefix);
        }

        // If found, erase the substring
        if (pos != std::string::npos) {
            if (modified_url.compare(pos, wss_prefix.length(), wss_prefix) == 0) {
                modified_url.erase(pos, wss_prefix.length());
            } else if (modified_url.compare(pos, ws_prefix.length(), ws_prefix) == 0) {
                modified_url.erase(pos, ws_prefix.length());
            }
        }

        return modified_url;
    }

    /// \brief Checks if a given URL starts with a specified scheme.
    /// \param url The URL string to check.
    /// \param scheme The scheme (e.g., "http") to check.
    /// \return True if the URL starts with the specified scheme, otherwise false.
    bool is_valid_scheme(const std::string& url, const std::string& scheme) {
        return url.compare(0, scheme.length(), scheme) == 0;
    }

    /// \brief Validates if a domain name is correctly formatted.
    /// \param domain The domain string to validate.
    /// \return True if the domain is valid, otherwise false.
    bool is_valid_domain(const std::string& domain) {
        size_t dot_pos = domain.find('.');

        if (dot_pos == std::string::npos ||
            dot_pos == 0 ||
            dot_pos == domain.length() - 1) {
            return false;
        }

        for (char ch : domain) {
            if (!isalnum(ch) && ch != '-' && ch != '.') {
                return false;
            }
        }

        std::string tld = domain.substr(dot_pos + 1);
        for (char ch : tld) {
            if (!isalpha(ch)) {
                return false;
            }
        }
        return true;
    }

    /// \brief Checks if a path is correctly formatted.
    /// \param path The path string to validate.
    /// \return True if the path is valid, otherwise false.
    bool is_valid_path(const std::string& path) {
        if (path.empty() || path[0] != '/') return false;
        for (char ch : path) {
            if (!isalnum(ch) && ch != '/' && ch != '-' && ch != '_') {
                return false;
            }
        }
        return true;
    }

    /// \brief Validates if a query string is correctly formatted.
    /// \param query The query string to validate.
    /// \return True if the query string is valid, otherwise false.
    bool is_valid_query(const std::string& query) {
        if (query.empty() || query[0] != '?') {
            return false;
        }

        size_t pos = 1;
        while (pos < query.length()) {
            size_t equalPos = query.find('=', pos);
            size_t ampPos = query.find('&', pos);

            if (equalPos == std::string::npos || (ampPos != std::string::npos && ampPos < equalPos)) {
                return false;
            }

            pos = (ampPos == std::string::npos) ? query.length() : ampPos + 1;
        }

        return true;
    }

    /// \brief Validates if a URL is correctly formatted.
    /// \param url The URL string to validate.
    /// \param protocols A vector of valid protocol schemes.
    /// \return True if the URL is valid, otherwise false.
    bool is_valid_url(const std::string& url, const std::vector<std::string>& protocol) {
        size_t scheme_end = url.find("://");
        if (scheme_end == std::string::npos) {
            return false;
        }

        const std::string scheme = url.substr(0, scheme_end);
        const auto it = std::find(protocol.begin(), protocol.end(), scheme);
        if (it == protocol.end()) return false;

        const size_t domain_start = scheme_end + 3;
        const size_t path_start = url.find('/', domain_start);
        const size_t query_start = url.find('?', domain_start);

        // Определяем конец домена
        size_t domain_end = (path_start != std::string::npos) ? path_start : query_start;
        if (domain_end == std::string::npos) {
            domain_end = url.length();
        }

        const std::string domain = url.substr(domain_start, domain_end - domain_start);
        if (!is_valid_domain(domain)) {
            return false;
        }

        // Проверяем путь, если он есть
        if (path_start != std::string::npos) {
            std::string path = (query_start != std::string::npos) ? url.substr(path_start, query_start - path_start) : url.substr(path_start);
            if (!is_valid_path(path)) {
                return false;
            }
        }

        // Проверяем query, если он есть
        if (query_start != std::string::npos) {
            std::string query = url.substr(query_start);
            if (!is_valid_query(query)) {
                return false;
            }
        }

        return true;
    }

    /// \brief Retrieves the directory of the executable file.
    /// \return A string containing the directory path of the executable.
    std::string get_exe_path() {
#       ifdef _WIN32
        std::vector<wchar_t> buffer(MAX_PATH);
        HMODULE hModule = GetModuleHandle(NULL);

        // Пробуем получить путь
        DWORD size = GetModuleFileNameW(hModule, buffer.data(), buffer.size());

        // Если путь слишком длинный, увеличиваем буфер
        while (size == buffer.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            buffer.resize(buffer.size() * 2);  // Увеличиваем буфер в два раза
            size = GetModuleFileNameW(hModule, buffer.data(), buffer.size());
        }

        if (size == 0) throw std::runtime_error("Failed to get executable path.");
        std::wstring exe_path(buffer.begin(), buffer.begin() + size);

        // Обрезаем путь до директории (удаляем имя файла, оставляем только путь к папке)
        size_t pos = exe_path.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            exe_path = exe_path.substr(0, pos);
        }

        // Преобразуем из std::wstring (UTF-16) в std::string (UTF-8)
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(exe_path);
#       else
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

        if (count == -1) {
            throw std::runtime_error("Failed to get executable path.");
        }

        std::string exe_path(result, count);

        // Обрезаем путь до директории (удаляем имя файла, оставляем только путь к папке)
        size_t pos = exe_path.find_last_of("\\/");
        if (pos != std::string::npos) {
            exe_path = exe_path.substr(0, pos);
        }

        return exe_path;
#   endif
    }

    /// \brief Thread-safe stream class for printing to console from multiple threads.
    class ThreadSafePrintStream : public std::ostringstream {
    public:
        ThreadSafePrintStream() = default;

        ~ThreadSafePrintStream() {
            get_instance().print(this->str());
        }

    private:
        class PrintStream {
        public:
            void print(const std::string &str) {
                std::lock_guard<std::mutex> guard(m_mutex_print);
                std::cout << str;
            }

        private:
            std::mutex m_mutex_print;
        };

        static PrintStream& get_instance() {
            static PrintStream instance; // Lazy initialization
            return instance;
        }
    };

}; // namespace utils
}; // namespace kurlyk

#endif // _KURLYK_UTILIS_HPP_INCLUDED
