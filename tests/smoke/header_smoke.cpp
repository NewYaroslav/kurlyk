#define KURLYK_AUTO_INIT 0
#define KURLYK_HTTP_SUPPORT 0
#define KURLYK_WEBSOCKET_SUPPORT 0

#include <kurlyk.hpp>

#include <iostream>
#include <string>

int main() {
    const std::string original = "Hello World!";
    const std::string encoded = kurlyk::utils::percent_encode(original);
    const std::string decoded = kurlyk::utils::percent_decode(encoded);

    if (encoded != "Hello%20World%21" || decoded != original) {
        std::cerr << "Percent encoding smoke check failed\n";
        return 1;
    }

    if (kurlyk::utils::remove_http_prefix("https://example.com") != "example.com") {
        std::cerr << "HTTP prefix smoke check failed\n";
        return 1;
    }

    kurlyk::QueryParams query = {{"q", "kurlyk smoke"}, {"page", "1"}};
    const std::string query_string = kurlyk::utils::to_query_string(query, "?");
    if (query_string.empty() || query_string[0] != '?') {
        std::cerr << "Query string smoke check failed\n";
        return 1;
    }

    return 0;
}
