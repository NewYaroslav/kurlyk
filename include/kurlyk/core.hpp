#pragma once
#ifndef _KURLYK_CORE_HPP_INCLUDED
#define _KURLYK_CORE_HPP_INCLUDED

/// \file core.hpp
/// \brief Aggregates core infrastructure components such as task manager interface and the network worker.

// Standard library
#include <atomic>
#include <chrono>
#include <string>
#include <cstring>
#include <functional>
#include <future>
#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <system_error>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

// Internal modules
#include "types.hpp"
#include "utils.hpp"

#include "core/INetworkTaskManager.hpp"
#include "core/NetworkWorker.hpp"

#endif // _KURLYK_CORE_HPP_INCLUDED
