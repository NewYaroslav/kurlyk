#pragma once
#ifndef _KURLYK_TYPES_HPP_INCLUDED
#define _KURLYK_TYPES_HPP_INCLUDED

/// \file types.hpp
/// \brief Aggregates type enumerations and utilities used throughout the Kurlyk library.

// Standard library
#include <string>
#include <unordered_map>
#include <stdexcept>

// Optional third-party integration
#if KURLYK_ENABLE_JSON
#include <nlohmann/json.hpp>
#endif

#if KURLYK_HTTP_SUPPORT
#include <curl/curl.h>
#endif

#include "utils/string_utils.hpp"

// Enumerations and conversion utilities
#include "types/enums.hpp"
#include "types/type_utils.hpp"
#include "types/Cookie.hpp"

#endif // _KURLYK_TYPES_HPP_INCLUDED
