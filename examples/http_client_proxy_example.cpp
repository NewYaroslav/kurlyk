#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <kurlyk.hpp>

/// \brief Reads proxy configuration from a text file.
/// \param filename Path to the configuration file.
/// \return A map containing proxy configuration values.
///
/// The configuration file format (proxy_config.txt) should be as follows:
/// ```
/// proxy_ip=127.0.0.1
/// proxy_port=8080
/// proxy_username=username
/// proxy_password=password
/// ```
/// Only `proxy_ip` and `proxy_port` are required. `proxy_username` and `proxy_password`
/// are optional and will only be used if present in the file.
std::unordered_map<std::string, std::string> read_proxy_config(const std::string& filename) {
    std::unordered_map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            config[key] = value;
        }
    }
    return config;
}

void print_response(const kurlyk::HttpResponsePtr& response) {
    KURLYK_PRINT
        << "ready: " << response->ready << std::endl
        << "response: " << std::endl
        << response->content << std::endl
        << "error_code: " << response->error_code << std::endl
        << "status_code: " << response->status_code << std::endl
        << "----------------------------------------" << std::endl;
}

int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    // Reading proxy parameters from configuration file
    const std::string config_filename = "proxy_config.txt";
    auto proxy_config = read_proxy_config(config_filename);

    // Setting proxy parameters if found in the configuration file
    if (proxy_config.find("proxy_ip") != proxy_config.end() &&
        proxy_config.find("proxy_port") != proxy_config.end()) {

        std::string proxy_ip = proxy_config["proxy_ip"];
        int proxy_port = std::stoi(proxy_config["proxy_port"]);
        std::string proxy_username = proxy_config["proxy_username"];
        std::string proxy_password = proxy_config["proxy_password"];

        client.set_proxy(proxy_ip, proxy_port, proxy_username, proxy_password, kurlyk::ProxyType::HTTP);
    }

    client.set_user_agent("KurlykClient/1.0");
    client.set_timeout(10);              // Setting request timeout in seconds
    client.set_connect_timeout(5);       // Setting connection timeout in seconds
    client.set_retry_attempts(3, 1000);  // Setting retries with delay of 1000 ms

    // Sending GET request using HttpClient
    KURLYK_PRINT << "Sending GET request using HttpClient..." << std::endl;
    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
               [](const kurlyk::HttpResponsePtr response) {
        print_response(response);
    });

    std::cin.get();
    client.cancel_requests();
    kurlyk::deinit();
    return 0;
}
