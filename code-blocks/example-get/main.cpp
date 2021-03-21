#include <iostream>
#include <kurlyk.hpp>


int main() {
    // Returns Origin IP.
    {
        std::cout << "Returns Origin IP, v1" << std::endl;
        kurlyk::Client client("https://httpbin.org");
        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.header = true;    // debug
        client.config.verbose = true;   // debug
        long err = client.request(
                "GET",              // method
                "/ip",              // path
                kurlyk::Arguments(),// args
                std::string(),      // content
                kurlyk::Headers(),  // headers
                [&](const kurlyk::Output &output) { // lambda

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "curl code: " << output.curl_code << std::endl;
            std::cout << "response code: " << output.response_code << std::endl;
        });
        std::cout << "error code: " << err << std::endl;
        std::cout << std::endl;
        std::system("pause");
    }
    // Returns Origin IP.
    {
        std::cout << "Returns Origin IP, v2" << std::endl;
        kurlyk::Client client("https://httpbin.org", true);
        std::cout << "response:" << std::endl << client.get("/ip", kurlyk::Headers()) << std::endl;
        std::cout << "response:" << std::endl << client.get("/ip") << std::endl;
        std::cout << std::endl;
        std::system("pause");
    }
    // Returns user-agent
    {
        std::cout << "Returns user-agent, v1" << std::endl;
        kurlyk::Client client("https://httpbin.org");
        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.user_agent = "curl kurlyk 1.0";
        long err = client.get("/user-agent", kurlyk::Headers(),
                [&](const kurlyk::Output &output) {
            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "curl code: " << output.curl_code << std::endl;
            std::cout << "response code: " << output.response_code << std::endl;
        });
        std::cout << "error code: " << err << std::endl;
        std::cout << std::endl;
        std::system("pause");
    }
    // Returns user-agent
    {
        std::cout << "Returns user-agent, v2" << std::endl;
        kurlyk::Client client("https://httpbin.org");
        client.config.set_default();
        client.get("/user-agent", kurlyk::Headers(),
                [&](const kurlyk::Output &output) {
            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "curl code: " << output.curl_code << std::endl;
            std::cout << "response code: " << output.response_code << std::endl;
        });
        std::cout << std::endl;
        std::system("pause");
    }
    {
        std::cout << "Returns header dict" << std::endl;
        kurlyk::Client client("https://httpbin.org");
        client.config.set_default();
        kurlyk::Headers headers;
        headers.emplace("Heron", "kurlyk-kurlyk");
        headers.emplace("Green", "elephant");
        client.add_cookie("author", "green-elephant");
        client.get("/headers", headers, [&](const kurlyk::Output &output) {
            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "curl code: " << output.curl_code << std::endl;
            std::cout << "response code: " << output.response_code << std::endl;
        });
        std::cout << std::endl;
        std::system("pause");
    }
    // Returns GET data
    {
        std::cout << "Returns GET data, v1" << std::endl;
        kurlyk::Client client("https://httpbin.org");

        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.user_agent = "Agent 007";
        client.config.timeout = 5;
        client.config.cookie = "elephant=Green";
        client.config.use_cookie = true;
        client.config.use_cookie_file = false;
        client.config.auto_referer = true;
        client.config.follow_location = true;
        client.config.max_redirects = 10;
        client.config.use_accept_encoding = true;
        client.config.use_brotli_encoding = true;
        client.config.use_deflate_encoding = false;
        client.config.use_gzip_encoding = false;
        client.config.use_identity_encoding = false;

        kurlyk::Headers headers;
        headers.emplace("Heron", "kurlyk-kurlyk");
        headers.emplace("Green", "elephant");

        long err = client.get("/get?heron=kurlyk-kurlyk", {{"Content-Type", "application/json"}}, [&](
                const kurlyk::Output &output) {
            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "curl code: " << output.curl_code << std::endl;
            std::cout << "response code: " << output.response_code << std::endl;
        });
        std::cout << "error code: " << err << std::endl;
        std::cout << std::endl;
        std::system("pause");
    }
    // Returns GET data
    {
        std::cout << "Returns GET data, v2" << std::endl;
        kurlyk::Client client("https://httpbin.org");

        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.user_agent = "Agent 007";
        client.config.timeout = 5;
        client.config.cookie = "elephant=Green";
        client.config.use_cookie = true;
        client.config.use_cookie_file = false;
        client.config.auto_referer = true;
        client.config.follow_location = true;
        client.config.max_redirects = 10;
        client.config.use_accept_encoding = true;
        client.config.use_brotli_encoding = true;
        client.config.use_deflate_encoding = false;
        client.config.use_gzip_encoding = false;
        client.config.use_identity_encoding = false;
        client.config.header = true;    // debug
        client.config.verbose = true;   // debug

        kurlyk::Headers headers;
        headers.emplace("Heron", "kurlyk-kurlyk");
        headers.emplace("Green", "elephant");

        long err = client.get(
                "/get",
                {{"heron", "kurlyk-kurlyk"}, {"elephant", "green"}},
                {{"Content-Type", "application/json"}},
                [&](const kurlyk::Output &output) {
            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "curl code: " << output.curl_code << std::endl;
            std::cout << "response code: " << output.response_code << std::endl;
        });
        std::cout << "error code: " << err << std::endl;
        std::cout << std::endl;
        std::system("pause");
    }
    // Sets a simple cookie
    {
        std::cout << "Sets a simple cookie, v1" << std::endl;
        kurlyk::Client client("https://httpbin.org");
        client.clear_cookie_file();

        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.cookie_file = "simple-cookie.txt";
        client.config.use_cookie = true;
        client.config.use_cookie_file = true;
        client.config.follow_location = true;
        client.config.header = true;
        client.config.verbose = true;

        client.get("/cookies/set/author/green",
                {
                    {"Connection", "keep-alive"},
                    {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
                },
                [&](const kurlyk::Output &output) {
            std::cout << "response:" << std::endl << output.response << std::endl;
        });
        client.get("/cookies", [&](const kurlyk::Output &output) {
            std::cout << "response:" << std::endl << output.response << std::endl;
        });
        std::cout << std::endl;
        std::system("pause");
    }
    // Sets a simple cookie
    {
        std::cout << "Sets a simple cookie, v2" << std::endl;
        kurlyk::Client client("https://httpbin.org");

        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.use_cookie = true;
        client.config.use_cookie_file = false;

        std::string response = client.get("/cookies/set/author/green");
        std::cout << "response:" << std::endl << response << std::endl;
        client.get("/cookies", [&](const kurlyk::Output &output) {
            std::cout << "response:" << std::endl << output.response << std::endl;
        });
        std::cout << std::endl;
        std::system("pause");
    }
    // Streams n–100 lines
    {
        std::cout << "Streams n–100 lines" << std::endl;
        kurlyk::Client client("https://httpbin.org");
        client.config.sert_file = "curl-ca-bundle.crt";
        client.get("/stream/100", [&](const kurlyk::Output &output) {
            std::cout << "response:" << std::endl << output.response << std::endl;
        });
        std::cout << std::endl;
        std::system("pause");
    }
    // Streams n–100 lines witch proxy
    {
        std::cout << "Streams n–100 lines witch proxy" << std::endl;
        kurlyk::Client client("https://httpbin.org");

        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.proxy_ip = "88.218.190.243";
        //client.config.proxy_port = 65233;
        client.config.proxy_port = 65234;
        client.config.proxy_username = "proxyfoxy";
        client.config.proxy_password = "12345";
        //client.config.proxy_type = kurlyk::ProxyTypes::HTTPS;
        client.config.proxy_type = kurlyk::ProxyTypes::SOCKS5;
        client.config.header = true;    // debug
        client.config.verbose = true;   // debug

        client.get("/stream/100", [&](const kurlyk::Output &output) {
            std::cout << "response:" << std::endl << output.response << std::endl;
        });
        std::cout << std::endl;
        std::system("pause");
    }
    return 0;
}
