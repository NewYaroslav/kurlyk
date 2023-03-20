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
#ifndef KURLYK_COMMON_HPP_INCLUDED
#define KURLYK_COMMON_HPP_INCLUDED

#include <curl/curl.h>

namespace kurlyk {

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
    }; // Output
}; // kurlyk

#endif // KURLYK_COMMON_HPP_INCLUDED
