#include <kurlyk.hpp>
#include <iostream>

int main() {
    try {

        // Example 1: Percent encode and decode
        std::string original = "Hello World! How are you?";
        std::string encoded = kurlyk::utils::percent_encode(original);
        std::string decoded = kurlyk::utils::percent_decode(encoded);
        std::cout << "Original: " << original << "\n";
        std::cout << "Encoded: " << encoded << "\n";
        std::cout << "Decoded: " << decoded << "\n";

        // Example 2: Validate email
        std::vector<std::string> emails = {"valid@example.com", "invalid-email", "user@domain.ruf"};
        for (const auto& email : emails) {
            std::cout << "Email: " << email << " is valid: " << std::boolalpha << kurlyk::utils::is_valid_email_id(email) << "\n";
        }

        // Example 3: Extract protocol from URL
        std::string url = "https://example.com/path?query=value";
        std::string protocol = kurlyk::utils::extract_protocol(url);
        std::cout << "Protocol in URL: " << protocol << "\n";

        // Example 4: Convert query parameters to a query string
        kurlyk::QueryParams query = {{"key1", "value1"}, {"key2", "value2"}};
        std::string query_string = kurlyk::utils::to_query_string(query);
        std::cout << "Query string: " << query_string << "\n";

        // Example 5: Validate domain and path
        std::string domain = "example.com";
        std::string path = "/valid/path";
        std::cout << "Domain " << domain << " is valid: " << std::boolalpha << kurlyk::utils::is_valid_domain(domain) << "\n";
        std::cout << "Path " << path << " is valid: " << std::boolalpha << kurlyk::utils::is_valid_path(path) << "\n";

        // Example 6: Validate URL
        std::vector<std::string> protocols = {"http", "https"};
        std::cout << "URL " << url << " is valid: " << std::boolalpha << kurlyk::utils::is_valid_url(url, protocols) << "\n";

        // Example 7: Convert User-Agent to sec-ch-ua
        std::string user_agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36";
        std::string sec_ch_ua = kurlyk::utils::convert_user_agent_to_sec_ch_ua(user_agent);
        std::cout << "sec-ch-ua header: " << sec_ch_ua << "\n";

        // Example 8: Get executable path
        std::string exe_path = kurlyk::utils::get_exec_dir();
        std::cout << "Executable path: " << exe_path << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
