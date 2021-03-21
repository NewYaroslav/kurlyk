#include <iostream>
#include <kurlyk.hpp>


int main() {
    // Returns Origin IP.
    {
        kurlyk::Client client("https://httpbin.org");
        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.header = true;
        client.config.verbose = true;
        client.request("GET", "/ip", std::string(), kurlyk::Headers(), [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {

            std::cout << "response: " << std::endl;
            std::cout << response << std::endl;
            std::cout << "err_code: " << err_code << std::endl;
            std::cout << "response_code: " << response_code << std::endl;
        });
    }
    // Returns user-agent
    {
        kurlyk::Client client("https://httpbin.org");
        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.user_agent = "kurlyk";
        client.get("/user-agent", kurlyk::Headers(), [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {

            std::cout << "response: " << std::endl;
            std::cout << response << std::endl;
            std::cout << "err_code: " << err_code << std::endl;
            std::cout << "response_code: " << response_code << std::endl;
        });
    }
    // Returns user-agent
    {
        kurlyk::Client client("https://httpbin.org");
        client.config.set_default();
        client.get("/user-agent", [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {

            std::cout << "response: " << std::endl;
            std::cout << response << std::endl;
            std::cout << "err_code: " << err_code << std::endl;
            std::cout << "response_code: " << response_code << std::endl;
        });
    }
    // Returns header dict.
    {
        kurlyk::Client client("https://httpbin.org");
        client.config.set_default();
        kurlyk::Headers headers;
        headers.emplace("Heron", "kurlyk-kurlyk");
        headers.emplace("Green", "elephant");
        client.add_cookie("author", "green-elephant");
        client.get("/headers", headers, [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {
            std::cout << response << std::endl;
        });
    }
    // Returns GET data
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.get("/get?heron=kurlyk-kurlyk", {{"Content-Type", "application/json"}}, [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {
            std::cout << response << std::endl;
        });
    }
    // Returns POST data
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.config.follow_location = false;
        client.post("/post", "{\"Heron\":\"kurlyk-kurlyk\"}", {{"Content-Type", "application/json"}}, [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {
            std::cout << response << std::endl;
        });
    }
    // Returns PUT data
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.config.follow_location = false;
        client.put("/put", "{\"Heron\":\"kurlyk-kurlyk\"}", {{"Content-Type", "application/json"}}, [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {
            std::cout << response << std::endl;
        });
    }
    // Returns PUT data
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.post("/gzip", "{\"Heron\":\"kurlyk-kurlyk\"}", {{"Content-Type", "application/json"}}, [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {
            std::cout << response << std::endl;
        });
    }
    // Sets a simple cookie
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.config.follow_location = true;
        client.config.header = true;
        client.config.verbose = true;
        client.get("/cookies/set/author/green",
                {
                    {"Connection", "keep-alive"},
                    {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
                    {"Accept-Language", "ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3"},
                    {"Accept-Encoding", "gzip, deflate, br"},
                    {"Upgrade-Insecure-Requests", "1"},
                }, [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {
            std::cout << response << std::endl;
        });
        client.get("/cookies", {{"Green", "elephant2"}}, [&](
                const kurlyk::Headers &headers,
                const std::string &response,
                const long err_code,
                const long response_code) {
            std::cout << response << std::endl;
        });
    }
    return 0;
}
