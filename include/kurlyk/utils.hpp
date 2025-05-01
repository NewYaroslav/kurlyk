#pragma once
#ifndef _KURLYK_UTILIS_HPP_INCLUDED
#define _KURLYK_UTILIS_HPP_INCLUDED

/// \file utils.hpp
/// \brief Contains utility functions and classes for handling HTTP requests and responses.

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>
#include <cctype>

#ifdef KURLYK_USE_CURL
#include <curl/curl.h>
#include "utils/CurlErrorCategory.hpp"
#endif

#include "utils/EventQueue.hpp"
#include "utils/CaseInsensitiveMultimap.hpp"

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

#include "utils/percent_encoding.hpp"
#include "utils/http_parser.hpp"
#include "utils/http_utils.hpp"
#include "utils/url_utils.hpp"
#include "utils/email_utils.hpp"
#include "utils/user_agent_utils.hpp"
#include "utils/print_utils.hpp"
#include "utils/path_utils.hpp"
#include "utils/encoding_utils.hpp"
#include "utils/string_utils.hpp"

#endif // _KURLYK_UTILIS_HPP_INCLUDED
