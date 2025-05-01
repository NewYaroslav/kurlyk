#pragma once
#ifndef _KURLYK_STARTUP_HPP_INCLUDED
#define _KURLYK_STARTUP_HPP_INCLUDED

/// \file startup.hpp
/// \brief Provides centralized startup and shutdown routines for the Kurlyk library.

#include "core.hpp"

#if KURLYK_HTTP_SUPPORT
#include "http.hpp"
#endif

#if KURLYK_WEBSOCKET_SUPPORT
#include "websocket.hpp"
#endif

#if defined(KURLYK_AUTO_INIT)
#include "startup/AutoInitializer.hpp"
#endif

#include "startup/runtime.hpp"

#endif // _KURLYK_STARTUP_HPP_INCLUDED
