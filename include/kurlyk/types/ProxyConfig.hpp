#pragma once
#ifndef _CRYPTOX_DATA_CONNECTION_PROXY_CONFIG_HPP_INCLUDED
#define _CRYPTOX_DATA_CONNECTION_PROXY_CONFIG_HPP_INCLUDED

/// \file ProxyConfig.hpp
/// \brief Defines the ProxyConfig structure for proxy server settings.

namespace cryptox {

    /// \struct ProxyConfig
    /// \brief Configuration structure for connecting through a proxy server.
    struct ProxyConfig {
        std::string proxy_server; ///< Proxy address in <ip:port> format.
        std::string proxy_auth;   ///< Proxy authentication in <username:password> format.
        ProxyType   proxy_type;   ///< Proxy type (e.g., HTTP, SOCKS5).
		bool        use = false;  ///< Whether to use proxy for connection.

        /// \brief Default constructor.
        ProxyConfig()
            : proxy_type(ProxyType::HTTP) {}

        /// \brief Constructs ProxyConfig with server and authentication.
        /// \param server Proxy server address.
        /// \param auth Proxy authentication credentials.
		/// \param type Proxy type.
        ProxyConfig(std::string server, std::string auth, ProxyType type)
            : proxy_server(std::move(server)), proxy_auth(std::move(auth)), proxy_type(type) {}

        /// \brief Sets the proxy server address.
        /// \param ip Proxy server IP address.
        /// \param port Proxy server port.
		/// \param type Proxy type.
        void set_proxy(const std::string& ip, int port, ProxyType type) {
            proxy_server = ip + ":" + std::to_string(port);
			proxy_type = type;
        }

        /// \brief Sets the proxy server address with authentication details and proxy type.
        /// \param ip Proxy server IP address.
        /// \param port Proxy server port.
        /// \param username Proxy username.
        /// \param password Proxy password.
        /// \param type Proxy type.
        void set_proxy(
            const std::string& ip,
            int port,
            const std::string& username,
            const std::string& password,
            ProxyType type = ProxyType::HTTP) {
            set_proxy(ip, port);
            set_proxy_auth(username, password);
            proxy_type = type;
        }

        /// \brief Sets proxy authentication credentials.
        /// \param username Proxy username.
        /// \param password Proxy password.
        void set_proxy_auth(const std::string& username, const std::string& password) {
            proxy_auth = username + ":" + password;
        }
		
		/// \brief Returns the proxy IP address (host part).
		/// \return Proxy IP as string, or empty string if not set or invalid.
		std::string get_ip() const {
			auto pos = proxy_server.find(':');
			if (pos == std::string::npos) return {};
			return proxy_server.substr(0, pos);
		}

		/// \brief Returns the proxy port.
		/// \return Proxy port as integer, or 0 if invalid.
		int get_port() const {
			auto pos = proxy_server.find(':');
			if (pos == std::string::npos) return 0;
			try {
				return std::stoi(proxy_server.substr(pos + 1));
			} catch (...) {
				return 0;
			}
		}

		/// \brief Returns the proxy username.
		/// \return Username string, or empty if not set.
		std::string get_username() const {
			auto pos = proxy_auth.find(':');
			if (pos == std::string::npos) return {};
			return proxy_auth.substr(0, pos);
		}

		/// \brief Returns the proxy password.
		/// \return Password string, or empty if not set.
		std::string get_password() const {
			auto pos = proxy_auth.find(':');
			if (pos == std::string::npos || pos + 1 >= proxy_auth.size()) return {};
			return proxy_auth.substr(pos + 1);
		}

        /// \brief Checks if the proxy configuration is valid (server field must not be empty).
        /// \return True if proxy_server is non-empty, otherwise false.
        bool is_valid() const {
            return !proxy_server.empty() && !proxy.get_ip().empty() && proxy.get_port() != 0;
        }
    };

    /// \brief Serializes ProxyConfig to JSON.
    inline void to_json(nlohmann::json& j, const ProxyConfig& config) {
        j = nlohmann::json{
            {"proxy_server", config.proxy_server},
            {"proxy_auth", config.proxy_auth},
            {"proxy_type", config.proxy_type},
			{"use", config.use}
        };
    }

    /// \brief Deserializes ProxyConfig from JSON.
    inline void from_json(const nlohmann::json& j, ProxyConfig& config) {
        if (j.contains("proxy_server")) config.proxy_server = j.at("proxy_server").get<std::string>();
        if (j.contains("proxy_auth"))   config.proxy_auth   = j.at("proxy_auth").get<std::string>();
        if (j.contains("proxy_type"))   config.proxy_type   = j.at("proxy_type").get<ProxyType>();
		if (j.contains("use"))          config.use          = j.at("use").get<bool>();
    }

} // namespace cryptox

#endif // _CRYPTOX_DATA_CONNECTION_PROXY_CONFIG_HPP_INCLUDED