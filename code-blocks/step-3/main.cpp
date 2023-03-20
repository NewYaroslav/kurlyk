#include <iostream>
#include <kurlyk.hpp>
#include <thread>

int main() {
    // Returns Origin IP.
    {
        kurlyk::Client client("https://httpbin.org");
        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.header = false;
        client.config.verbose = false;
        //
        client.loop();
        //
        for (int i = 0; i < 10; ++i) {
            client.async_request("GET", "/ip", kurlyk::Arguments(), std::string(), kurlyk::Headers(),
                    [i](const kurlyk::Output &output) {
                std::cout << "& <" << i << ">" << std::endl;
                std::cout << "response: " << std::endl;
                std::cout << output.response << std::endl;
                std::cout << "err_code: " << output.curl_code << std::endl;
                std::cout << "response_code: " << output.response_code << std::endl;
                std::cout << "----------------------------------------" << std::endl;
            });
        }
        std::cout << "loop" << std::endl;

        std::atomic<bool> reset = ATOMIC_VAR_INIT(false);

        std::thread t1([&reset, &client](){
            while(!reset) {
                client.loop();
            }
        });

        std::thread t2([&reset, &client](){
            for (int i = 0; i < 10; ++i) {
                client.async_request("GET", "/ip", kurlyk::Arguments(), std::string(), kurlyk::Headers(),
                        [i](const kurlyk::Output &output) {
                    std::cout << "- <" << i << ">" << std::endl;
                    std::cout << "response: " << std::endl;
                    std::cout << output.response << std::endl;
                    std::cout << "err_code: " << output.curl_code << std::endl;
                    std::cout << "response_code: " << output.response_code << std::endl;
                    std::cout << "----------------------------------------" << std::endl;
                });
                client.async_request("GET", "/user-agent", kurlyk::Arguments(), std::string(), kurlyk::Headers(),
                        [i](const kurlyk::Output &output) {
                    std::cout << "- <" << i << ">" << std::endl;
                    std::cout << "response: " << std::endl;
                    std::cout << output.response << std::endl;
                    std::cout << "err_code: " << output.curl_code << std::endl;
                    std::cout << "response_code: " << output.response_code << std::endl;
                    std::cout << "----------------------------------------" << std::endl;
                });
            }
        });

        for (int i = 0; i < 10; ++i) {
            client.async_request("GET", "/headers", kurlyk::Arguments(), std::string(), kurlyk::Headers(),
                    [i](const kurlyk::Output &output) {
                std::cout << "+ <" << i << ">" << std::endl;
                std::cout << "response: " << std::endl;
                std::cout << output.response << std::endl;
                std::cout << "err_code: " << output.curl_code << std::endl;
                std::cout << "response_code: " << output.response_code << std::endl;
                std::cout << "----------------------------------------" << std::endl;
            });
            client.async_request("GET", "/user-agent", kurlyk::Arguments(), std::string(), kurlyk::Headers(),
                    [i](const kurlyk::Output &output) {
                std::cout << "+ <" << i << ">" << std::endl;
                std::cout << "response: " << std::endl;
                std::cout << output.response << std::endl;
                std::cout << "err_code: " << output.curl_code << std::endl;
                std::cout << "response_code: " << output.response_code << std::endl;
                std::cout << "----------------------------------------" << std::endl;
            });
        }

        reset = true;
        std::cout << "-1-running_handles: " << client.get_running_handles() << std::endl;
        t1.join();
        t2.join();
        std::cout << "-2-running_handles: " << client.get_running_handles() << std::endl;
    }
    //return 0;
    // Returns Origin IP.
    {
        kurlyk::Client client("https://httpbin.org");
        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.header = true;
        client.config.verbose = true;
        client.request("GET", "/ip", kurlyk::Arguments(), std::string(), kurlyk::Headers(),
                [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
        });
    }
    // Returns user-agent
    {
        kurlyk::Client client("https://httpbin.org");
        client.config.sert_file = "curl-ca-bundle.crt";
        client.config.user_agent = "kurlyk";
        client.get("/user-agent", kurlyk::Headers(),
                [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
        });
    }
    // Returns user-agent
    {
        kurlyk::Client client("https://httpbin.org");
        client.config.set_default();
        client.get("/user-agent", [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
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
        client.get("/headers", headers, [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
        });
    }
    // Returns GET data
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.get("/get?heron=kurlyk-kurlyk", {{"Content-Type", "application/json"}}, [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
        });
    }
    // Returns POST data
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.config.follow_location = false;
        client.post("/post", "{\"Heron\":\"kurlyk-kurlyk\"}", {{"Content-Type", "application/json"}}, [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
        });
    }
    // Returns PUT data
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.config.follow_location = false;
        client.put("/put", "{\"Heron\":\"kurlyk-kurlyk\"}", {{"Content-Type", "application/json"}}, [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
        });
    }
    // Returns PUT data
    {
        kurlyk::Client client("https://httpbin.org", true);
        client.post("/gzip", "{\"Heron\":\"kurlyk-kurlyk\"}", {{"Content-Type", "application/json"}}, [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
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
                }, [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
        });
        client.get("/cookies", {{"Green", "elephant2"}}, [&](const kurlyk::Output &output) {

            std::cout << "response: " << std::endl;
            std::cout << output.response << std::endl;
            std::cout << "err_code: " << output.curl_code << std::endl;
            std::cout << "response_code: " << output.response_code << std::endl;
        });
    }
    return 0;
}
