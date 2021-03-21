/*
* kurlyk - C ++ library for easy curl work
*
* Copyright (c) 2021 Elektro Yar. Email: git.electroyar@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#pragma once
#ifndef KURLYK_UTILITY_HPP_INCLUDED
#define KURLYK_UTILITY_HPP_INCLUDED

#ifdef KYRLUK_AES_SUPPORT
#include "kurlyk-file-encryption.hpp"
#endif

#include <unordered_map>
#include <vector>
#include <curl/curl.h>

namespace kurlyk {
	namespace utility {

        /** \brief Функция сравнения строк для CaseInsensitiveMultimap
         *
         * Оригинал тут: https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/master/utility.hpp#L42
         * Спасибо Ole Christian Eidheim за библиотеку!
         */
        inline bool case_insensitive_equal(const std::string &str1, const std::string &str2) noexcept {
            return str1.size() == str2.size() &&
               std::equal(str1.begin(), str1.end(), str2.begin(), [](char a, char b) {
                 return tolower(a) == tolower(b);
               });
        }

        /** \brief Оператор сравнения строк для CaseInsensitiveMultimap
         *
         * Оригинал тут: https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/master/utility.hpp#L48
         * Спасибо Ole Christian Eidheim за библиотеку!
         */
        class CaseInsensitiveEqual {
        public:
            bool operator()(const std::string &str1, const std::string &str2) const noexcept {
                return case_insensitive_equal(str1, str2);
            }
        };

        // Based on https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226

        /** \brief Hash функция для CaseInsensitiveMultimap
         *
         * Основано на: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226
         * Оригинал тут: https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/master/utility.hpp#L55
         * Спасибо Ole Christian Eidheim за библиотеку!
         */
        class CaseInsensitiveHash {
        public:
            std::size_t operator()(const std::string &str) const noexcept {
                std::size_t h = 0;
                std::hash<int> hash;
                for(auto c : str)
                    h ^= hash(tolower(c)) + 0x9e3779b9 + (h << 6) + (h >> 2);
                return h;
            }
        };

        using CaseInsensitiveMultimap = std::unordered_multimap<std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEqual>;

        /** \brief Класс для хранения данных Cookie
         */
        class Cookie {
        public:
            std::string name;
            std::string value;
            std::string path;
            uint64_t expiration_date = 0;
        };

        using CaseInsensitiveCookieMultimap = std::unordered_multimap<std::string, Cookie, CaseInsensitiveHash, CaseInsensitiveEqual>;

        CaseInsensitiveCookieMultimap parse_cookie(std::string cookie) noexcept {
            CaseInsensitiveCookieMultimap temp;
            std::vector<std::string> list_fragment;
            cookie += ";";
            const std::string secure_prefix("__Secure-");
            const std::string host_prefix("__Host-");
            std::size_t start_pos = 0;
            while(true) {
                bool is_option = false;
                std::size_t found_separator = cookie.find_first_of("=;", start_pos);
                if(found_separator != std::string::npos) {
                    std::string name = cookie.substr(start_pos, (found_separator - start_pos));

                    if(name.size() > host_prefix.size() && name.substr(0,2) == "__") {

                        std::size_t found_separator_prefix = name.find_first_of("-");
                        if(found_separator_prefix != std::string::npos) {
                            name = name.substr(found_separator_prefix + 1, name.size() - found_separator_prefix - 1);
                            std::cout << "-3 name = " << name << std::endl;
                        }
                    } else {
                        if (case_insensitive_equal(name, "expires") ||
                            case_insensitive_equal(name, "max-age") ||
                            case_insensitive_equal(name, "path") ||
                            case_insensitive_equal(name, "domain") ||
                            case_insensitive_equal(name, "samesite")) {
                            is_option = true;
                        } else
                        if (case_insensitive_equal(name, "secure") ||
                            case_insensitive_equal(name, "httponly")) {
                            if(cookie[found_separator ] == ';') start_pos = found_separator + 2;
                            else start_pos = found_separator + 1;
                            continue;
                        }
                    }

                    start_pos = cookie.find("; ", found_separator + 1);
                    if(start_pos == std::string::npos) {
                        std::string value = cookie.substr(found_separator + 1, cookie.size() - found_separator - 2);
                        Cookie cookie;
                        cookie.name = name;
                        cookie.value = value;
                        if(!is_option) temp.emplace(cookie.name, cookie);
                        break;
                    }
                    std::string value = cookie.substr(found_separator + 1, start_pos - found_separator - 1);
                    start_pos += 2;
                    Cookie cookie;
                    cookie.name = name;
                    cookie.value = value;
                    if(!is_option) temp.emplace(cookie.name, cookie);
                } else {
                    break;
                }
            }
            return std::move(temp);
        };

        /** \brief Класс для хранения HTTP заголовков библиотеки CURL
         */
        class CurlHeaders {
        private:
            struct curl_slist *http_headers = nullptr;
        public:

            CurlHeaders() noexcept {};

            CurlHeaders(const std::vector<std::pair<std::string, std::string>> &headers) noexcept {
                for(size_t i = 0; i < headers.size(); ++i) {
                    add_header(headers[i].first, headers[i].second);
                }
            };

            CurlHeaders(const std::vector<std::string> &headers) noexcept {
                for(size_t i = 0; i < headers.size(); ++i) {
                    add_header(headers[i]);
                }
            };

            CurlHeaders(const CaseInsensitiveMultimap &headers) noexcept {
                for(auto &header_field : headers) {
                    add_header(header_field.first, header_field.second);
                }
            };

            void add_header(const std::string &header) noexcept {
                http_headers = curl_slist_append(http_headers, header.c_str());
            }

            void add_header(const std::string &key, const std::string &val) noexcept {
                std::string header(key + ": " + val);
                http_headers = curl_slist_append(http_headers, header.c_str());
            }

            ~CurlHeaders() noexcept {
                if(http_headers != nullptr) {
                    curl_slist_free_all(http_headers);
                    http_headers = nullptr;
                }
            };

            inline struct curl_slist *get_curl() const noexcept {
                return http_headers;
            }

            inline bool empty() const noexcept {
                return (http_headers == nullptr);
            }
        };
	}; // utility
}; // kurlyk

#endif // KURLYK_UTILITY_HPP_INCLUDED
