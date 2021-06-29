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
#ifndef KYRLUK_HPP_INCLUDED
#define KYRLUK_HPP_INCLUDED

#include "kurlyk-utility.hpp"
#include <curl/curl.h>
#include <mutex>
#include <atomic>
#include <functional>

#ifdef KYRLUK_BROTLI_SUPPORT
#include <brotli/decompress.hpp>
#endif

#ifdef KYRLUK_GZIP_SUPPORT
#include <gzip/decompress.hpp>
#endif

namespace kurlyk {

    using Headers = utility::CaseInsensitiveMultimap;
    using Arguments = utility::CaseInsensitiveMultimap;
    using Cookies = utility::CaseInsensitiveCookieMultimap;
    using Cookie = utility::Cookie;

    enum ErrorCodes {
        OK = 0,
        NO_ANSWER = -1000,
        CONTENT_ENCODING_NOT_SUPPORT = -1001,
        CURL_REQUEST_FAILED = -1002,
    };

    enum class ProxyTypes {
        HTTP = CURLPROXY_HTTP,
        HTTPS = CURLPROXY_HTTPS,
        HTTP_1_0 = CURLPROXY_HTTP_1_0,
        SOCKS4 = CURLPROXY_SOCKS4,
        SOCKS4A = CURLPROXY_SOCKS4A,
        SOCKS5 = CURLPROXY_SOCKS5,
        SOCKS5_HOSTNAME = CURLPROXY_SOCKS5_HOSTNAME
    };

    class Output {
    public:
        std::string response;
        Headers headers;
        long curl_code;
        long response_code;
    };

    /** \brief Класс для создания запроса
     */
    class Client {
    public:

        /** \brief Класс для хранения конфигурации соединения
         */
        class Config {
        public:
            /* параметры подключения */
            std::string user_agent;
            std::string sert_file;
            std::string cookie_file;
            std::string cookie;
            bool use_cookie = true;
            bool use_cookie_file = true;
            bool use_accept_encoding = true;
            bool use_deflate_encoding = true;
            bool use_gzip_encoding = true;
            bool use_brotli_encoding = true;
            bool use_identity_encoding = false;

            bool follow_location = true;
            long max_redirects = 10;
            bool auto_referer = false;

            /* параметры прокси */
            std::string proxy_ip;
            std::string proxy_username;
            std::string proxy_password;
            int proxy_port = 0;
            ProxyTypes proxy_type = ProxyTypes::HTTP;
            bool use_proxy_tunnel = true;

            /* параметры отладки */
            bool verbose = false;
            bool header = false;

            int timeout = 5;

#           ifdef KYRLUK_AES_SUPPORT
            std::array<uint8_t, 32> key;
            std::string cookie_protected_file;
#           endif

            inline void set_default() noexcept {
                user_agent = "Mozilla/5.0 (Windows NT 6.3; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.190 Safari/537.36";
                cookie_file = "curl-cookie.txt";
#               ifdef KYRLUK_AES_SUPPORT
                cookie_protected_file = "cookie.dat";
#               endif
                sert_file = "curl-ca-bundle.crt";
                timeout = 5;
            }
        };

        Config config;  /**< Настройки подключения */

        /** \brief Callback-функция для обработки ответа
         * Данная функция нужна для внутреннего использования
         */
        static int http_request_writer(char *data, size_t size, size_t nmemb, void *userdata) {
            int result = 0;
            std::string *buffer = (std::string*)userdata;
            if(buffer != NULL) {
                buffer->append(data, size * nmemb);
                result = size * nmemb;
            }
            return result;
        }

        inline static void http_request_parse_pair(const std::string &value, std::string &one, std::string &two) noexcept {
            if(value.size() < 3) return;
            std::size_t start_pos = 0;
            std::size_t found_beg = value.find_first_of(":", 0);
            std::size_t found_end = value.find_first_not_of(" ", found_beg + 1);
            if(found_beg != std::string::npos) {
                std::size_t len = found_beg - start_pos;
                if(len > 0) {
                    one = value.substr(start_pos, len);
                    start_pos = one.size() + 1;
                    len = value.size() - start_pos;
                    two = value.substr(found_end);
                }
            }
        }

        /** \brief Callback-функция для обработки HTTP Header ответа
         *
         * Данный метод нужен, чтобы определить, какой тип сжатия данных используется (или сжатие не используется)
         * Данный метод нужен для внутреннего использования
         */
        static int http_request_header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
            size_t buffer_size = nitems * size;
            Headers *headers =
                (Headers*)userdata;
            std::pair<std::string, std::string> key_value;
            http_request_parse_pair(
                std::string(buffer, buffer_size),
                key_value.first,
                key_value.second);
            headers->emplace(key_value.first, key_value.second);
            return buffer_size;
        }

    private:
        char error_buffer[CURL_ERROR_SIZE];
        std::string host;
        Cookies cookies_buffer;
        std::mutex cookies_buffer_mutex;
        std::atomic<bool> is_clear_cookie_file = ATOMIC_VAR_INIT(false);

        using CurlHeaders = utility::CurlHeaders;

        /** \brief Инициализация CURL
         *
         * Данный метод является общей инициализацией для всех видов запросов
         * Данный метод нужен для внутреннего использования
         * \param method            Параметры curl
         * \param path
         * \param content
         * \param headers
         * \param response          Ответ сервера
         * \param writer_callback   Callback-функция для записи данных от сервера
         * \param header_callback   Callback-функция для обработки заголовков ответа
         * \param header_data       Данные заголовков
         * \return вернет указатель на CURL или NULL, если инициализация не удалась
         */
        CURL *init_curl(
                const std::string &method,
                const std::string &path,
                const std::string &args,
                const std::string &content,
                //const std::string &cookie,
                const Headers &headers,
                const CurlHeaders &curl_headers,
                std::string &response,
                int (*writer_callback)(char*, size_t, size_t, void*),
                int (*header_callback)(char*, size_t, size_t, void*),
                void *header_data) {
            CURL *curl = curl_easy_init();
            if(!curl) return NULL;

            std::string url(host);
            if(!path.empty() && path[0] != '/') url += "/";
            url += path;
            if(!args.empty()) url += args;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
            curl_easy_setopt(curl, CURLOPT_CAINFO, config.sert_file.c_str());
            curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_DEFAULT);

            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, config.follow_location);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, config.max_redirects);
            curl_easy_setopt(curl, CURLOPT_AUTOREFERER, config.auto_referer);

            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout);

            if (config.use_accept_encoding) {
                std::string accept_encoding;
                if(config.use_identity_encoding) accept_encoding += "identity";
                if(config.use_brotli_encoding) {if(!accept_encoding.empty()) accept_encoding += ", "; accept_encoding += "br";}
                if(config.use_gzip_encoding) {if(!accept_encoding.empty()) accept_encoding += ", "; accept_encoding += "gzip";}
                if(config.use_deflate_encoding) {if(!accept_encoding.empty()) accept_encoding += ", "; accept_encoding += "deflate";}
                curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, accept_encoding.c_str());
            }

            if (!config.proxy_ip.empty()) {
                const std::string proxy(config.proxy_ip + ":" + std::to_string(config.proxy_port));
                curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
                curl_easy_setopt(curl, CURLOPT_PROXYTYPE, static_cast<long>(config.proxy_type));
                curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, config.use_proxy_tunnel);
            }

            if (!config.proxy_username.empty() && !config.proxy_password.empty()) {
                const std::string proxy_auth(config.proxy_username + ":" + config.proxy_password);
                curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxy_auth.c_str());

                /* CURLOPT_HTTPAUTH - параметр этой опции определяет тип HTTP аутентификации, возможные значения:
                 * CURLAUTH_BASIC - базовый вид аутентификации, используемый, как правило, по умолчанию. Имя пользователя и пароль передаются открытым текстом.
                 * CURLAUTH_DIGEST - является более безопасной версией предыдущей.
                 * CURLAUTH_ANY - этот параметр заставляет libcurl выбрать все типы аутентификации, срабатывает метод наиболее подходящий к данному случаю
                 */
                curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
                //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                // curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 1);
                //curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
            }

            /* настроим Cookie */
            if (config.use_cookie && headers.find("Cookie") == headers.end()) {
                if(config.use_cookie_file) {
                    if(!config.cookie_file.empty()) {
                        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, config.cookie_file.c_str());
                        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, config.cookie_file.c_str());
                        if(is_clear_cookie_file) curl_easy_setopt(curl, CURLOPT_COOKIELIST, "ALL");
                    }
                } else {
                    const std::string cookie_buffer = get_str_cookie_buffer();
                    //if(!cookie.empty()) curl_easy_setopt(curl, CURLOPT_COOKIE, cookie.c_str());
                    if(!cookie_buffer.empty()) curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_buffer.c_str());
                    else
                    if(!config.cookie.empty()) curl_easy_setopt(curl, CURLOPT_COOKIE, config.cookie.c_str());
                }
            }
            is_clear_cookie_file = false;

            /* настроим User Agent */
            if (!config.user_agent.empty() && headers.find("User-Agent") == headers.end()) {
                curl_easy_setopt(curl, CURLOPT_USERAGENT, config.user_agent.c_str());
            }

            /* настроим работу с Headers */
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, header_data);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
            if(!headers.empty()) {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers.get_curl());
            }

            /* настроим тело запроса */
            if (utility::case_insensitive_equal(method, "POST") ||
                utility::case_insensitive_equal(method, "PUT") ||
                utility::case_insensitive_equal(method, "DELETE")) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
            }

            /* настроим параметры отладки */
            curl_easy_setopt(curl, CURLOPT_VERBOSE, config.verbose);
            curl_easy_setopt(curl, CURLOPT_HEADER, config.header);
            return curl;
        }

        std::string get_str_cookie_buffer() noexcept {
            std::string temp;
            size_t index = 0;
            {
                std::lock_guard<std::mutex> lock(cookies_buffer_mutex);
                const size_t index_end = cookies_buffer.size() - 1;
                for(auto &c : cookies_buffer) {
                    temp += c.second.name;
                    temp += "=";
                    temp += c.second.value;
                    if(index != index_end) temp += "; ";
                    ++index;
                }
            }
            return std::move(temp);
        }

    public:

        Client(const std::string &user_host, const bool use_default = false) noexcept : host(user_host) {
            curl_global_init(CURL_GLOBAL_ALL);
            if(use_default) config.set_default();
#           ifdef KYRLUK_AES_SUPPORT
            const uint32_t seed = 20032021;
            config.key = utility::get_generated_key(seed);
#           endif
        }

        /** \brief Получить строку из аргументов
         * \param args  Аргументы
         * \return Вернет строку
         */
        std::string get_str_args(const Arguments &args) noexcept {
            if (args.empty()) return std::string();
            std::string temp("?");
            CURL *curl = curl_easy_init();
            size_t index = 0;
            const size_t index_end = args.size() - 1;
            for(auto &a : args) {
                char *output_key = curl_easy_escape(curl, a.first.c_str(), a.first.size());
                if(output_key) {
                    temp += std::string(output_key);
                    curl_free(output_key);
                }
                temp += "=";
                char *output_value = curl_easy_escape(curl, a.second.c_str(), a.second.size());
                if(output_value) {
                    temp += std::string(output_value);
                    curl_free(output_value);
                }
                if(index != index_end) temp += "&";
                ++index;
            }
            curl_easy_cleanup(curl);
            return std::move(temp);
        }

        /** \brief Очистить файл Сookie
         * Данный метод устанавливает флаг очистки файла Cookie
         * В качестве файла Сookie используется файл для работы с cURL
         */
        inline void clear_cookie_file() noexcept {
            is_clear_cookie_file = true;
        }

        /** \brief Очистить данные Сookie
         * \return Вернет true, если данные Сookie удалены
         */
        inline bool clear_cookie_buffer() noexcept {
            std::lock_guard<std::mutex> lock(cookies_buffer_mutex);
            if(cookies_buffer.empty()) return false;
            cookies_buffer.clear();
            return true;
        }

        inline void add_cookie(const Cookie &cookie) noexcept {
            std::lock_guard<std::mutex> lock(cookies_buffer_mutex);
            cookies_buffer.emplace(cookie.name, cookie);
        }

        inline void add_cookie(const std::string &name, const std::string &value) noexcept {
            Cookie cookie;
            cookie.name = name;
            cookie.value = value;
            add_cookie(cookie);
        }

#       ifdef KYRLUK_AES_SUPPORT
        /** \brief Очистить файл Сookie
         * \return Вернет true, если файл очищен
         */
        bool clear_protected_file_cookie() const noexcept {
            {
                std::lock_guard<std::mutex> lock(cookies_buffer_mutex);
                cookies_buffer.clear();
            }
            if(config.cookie_protected_file.empty()) return false;
            nlohmann::json j_data;
            j_data["cookies"] = nlohmann::json::array();
            return save_encrypt_file(config.cookie_protected_file, j_data, config.key);
        }

        /** \brief Сохранить файл Сookie
         * \return Вернет true, если запись прошла успешно
         */
        bool save_protected_file_cookie() const noexcept {
            if(config.cookie_protected_file.empty()) return false;
            nlohmann::json j_data;
            nlohmann::json j_cookies = nlohmann::json::array();
            size_t index = 0;
            {
                std::lock_guard<std::mutex> lock(cookies_buffer_mutex);
                for(auto &c : cookies_buffer) {
                    j_cookies[index]["name"] = c.second.name;
                    j_cookies[index]["value"] = c.second.value;
                    j_cookies[index]["path"] = c.second.path;
                    j_cookies[index]["expiration_date"] = c.second.expiration_date;
                    ++index;
                }
            }
            j_data["cookies"] = j_cookies;
            return utility::save_encrypt_file(config.cookie_protected_file, j_data, config.key);
        }

        bool open_protected_file_cookie() const noexcept {
            if(config.cookie_protected_file.empty()) return false;
            nlohmann::json j_data;
            if(!utility::open_encrypt_file(config.cookie_protected_file, j_data, config.key)) return false
            nlohmann::json j_cookies = j_data["cookies"];
            {
                std::lock_guard<std::mutex> lock(cookies_buffer_mutex);
                for(size_t index = 0; i < j_cookies.size(); ++i) {
                    Cookie cookie;
                    cookie.name = j_cookies[index]["name"];
                    cookie.value = j_cookies[index]["value"];
                    cookie.path = j_cookies[index]["path"];
                    cookie.expiration_date = j_cookies[index]["expiration_date"];
                    cookies_buffer.emplace(cookie.name, cookie);
                }
            }
            return true;
        }
#       endif

        long request(
                const std::string &method,
                const std::string &path,
                const Arguments &args,
                const std::string &content,
                const Headers &headers,
                const std::function<void(const Output &output)> &callback) {

            Headers response_headers;
            std::string response_buffer;

            CurlHeaders curl_headers(headers);
            const std::string args_str = get_str_args(args);
            //const std::string cookie_buffer_str = get_str_cookie_buffer();

            CURL *curl = init_curl(
                method,
                path,
                args_str,
                content,
                //cookie_buffer_str,
                headers,
                curl_headers,
                response_buffer,
                http_request_writer,
                http_request_header_callback,
                &response_headers);

            Output output;
            output.curl_code = curl_easy_perform(curl);
            //CURLcode curl_code = curl_easy_perform(curl);
            output.response_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &output.response_code);
            curl_easy_cleanup(curl);

            if(output.curl_code != CURLE_OK) {
                output.response.swap(response_buffer);
                output.headers.swap(response_headers);
                callback(output);
                return output.response_code;
            }

            /* сохраняем Cookie */
            if(config.use_cookie && !config.use_cookie_file) {
                auto header_it = response_headers.find("Set-Cookie");
                if(header_it != response_headers.end()) {
                    Cookies set_cookie_data = utility::parse_cookie(header_it->second);
                    std::lock_guard<std::mutex> lock(cookies_buffer_mutex);
                    cookies_buffer.insert(set_cookie_data.begin(), set_cookie_data.end());
                }
            }

            if(!config.use_accept_encoding) {
                auto header_it = response_headers.find("Content-Encoding");
                if(header_it != response_headers.end()) {
                    if(!response_buffer.empty()) {
                        const std::string &content_encoding = header_it->second;
                        if(content_encoding.find("gzip") != std::string::npos) {
#                           ifdef KYRLUK_GZIP_SUPPORT
                            const char *compressed_pointer = response_buffer.data();
                            output.response = gzip::decompress(compressed_pointer, response_buffer.size());
#                           else
                            output.curl_code = static_cast<CURLcode>(CONTENT_ENCODING_NOT_SUPPORT);
                            output.response.swap(response_buffer);
#                           endif
                        } else
                        if(content_encoding.find("br") != std::string::npos) {
#                           ifdef KYRLUK_BROTLI_SUPPORT
                            output.response = gzip::decompress(response_buffer);
#                           else
                            output.curl_code = static_cast<CURLcode>(CONTENT_ENCODING_NOT_SUPPORT);
                            output.response.swap(response_buffer);
#                           endif
                        } else
                        if(content_encoding.find("identity") != std::string::npos) {
                            output.response.swap(response_buffer);
                        } else {
                            if(output.response_code == 200) output.curl_code = static_cast<CURLcode>(CONTENT_ENCODING_NOT_SUPPORT);
                            else output.curl_code = static_cast<CURLcode>(CURL_REQUEST_FAILED);
                            output.response.swap(response_buffer);
                        }
                    }
                } else {
                    output.response.swap(response_buffer);
                }
            } else {
                output.response.swap(response_buffer);
            }

            if(callback != nullptr) {
                callback(output);
            }
            return static_cast<long>(output.curl_code);
        }

        long get(
                const std::string &path,
                const Arguments &args,
                const Headers &headers,
                const std::function<void(const Output &output)> &callback) noexcept {
            return request("GET", path, args, std::string(), headers, callback);
        }

        long get(
                const std::string &path,
                const Headers &headers,
                const std::function<void(const Output &output)> &callback) noexcept {
            return request("GET", path, Arguments(), std::string(), headers, callback);
        }

        long get(
                const std::string &path,
                const std::function<void(const Output &output)> &callback) noexcept {
            return request("GET", path, Arguments(), std::string(), Headers(), callback);
        }

        std::string get(const std::string &path, const Arguments &args, const Headers &headers) {
            std::string response;
            request("GET", path, args, std::string(), headers,
                    [&](const Output &output){
                response = output.response;
            });
            return response;
        }

        std::string get(const std::string &path, const Headers &headers) {
            std::string response;
            request("GET", path, Arguments(), std::string(), headers,
                    [&](const Output &output){
                response = output.response;
            });
            return response;
        }

        std::string get(const std::string &path) {
            std::string response;
            request("GET", path, Arguments(), std::string(), Headers(),
                    [&](const Output &output){
                response = output.response;
            });
            return response;
        }

        long post(
                const std::string &path,
                const Arguments &args,
                const std::string &content,
                const Headers &headers,
                const std::function<void(const Output &output)> &callback) noexcept {
            return request("POST", path, args, content, headers, callback);
        }

        long post(
                const std::string &path,
                const std::string &content,
                const Headers &headers,
                const std::function<void(const Output &output)> &callback) noexcept {
            return request("POST", path, Arguments(), content, headers, callback);
        }

        long put(
                const std::string &path,
                const std::string &content,
                const Headers &headers,
                const std::function<void(const Output &output)> &callback) noexcept {
            return request("PUT", path, Arguments(), content, headers, callback);
        }
    };
};

#endif // KYRLUK_HPP_INCLUDED
