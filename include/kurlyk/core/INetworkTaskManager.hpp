#pragma once
#ifndef _KURLYK_CORE_INETWORKTASKMANAGER_HPP_INCLUDED
#define _KURLYK_CORE_INETWORKTASKMANAGER_HPP_INCLUDED

/// \file INetworkTaskManager.hpp
/// \brief Defines an interface for modules that can register with the NetworkWorker for lifecycle handling.

namespace kurlyk::core {

    /// \class INetworkTaskManager
    /// \brief Interface for modules managed by NetworkWorker (e.g., HTTP, WebSocket).
    class INetworkTaskManager {
    public:
        /// \brief Called periodically to process tasks.
        virtual void process() = 0;

        /// \brief Called during shutdown to clean up.
        virtual void shutdown() = 0;

        /// \brief Indicates whether the module has pending or active work.
        virtual const bool is_loaded() const = 0;

        virtual ~INetworkTaskManager() = default;
    };

} // namespace kurlyk::core

#endif // _KURLYK_CORE_INETWORKTASKMANAGER_HPP_INCLUDED
