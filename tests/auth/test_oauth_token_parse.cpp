#define KURLYK_JSON_SUPPORT 0
#include <kurlyk.hpp>
#include <string>

struct TestClient : kurlyk::http::auth::OAuthPkceClient {
    TestClient(const kurlyk::OAuthConfig& cfg) : kurlyk::http::auth::OAuthPkceClient(cfg) {}
    bool test_parse(const std::string& raw, kurlyk::AuthResult& out) {
        return parse_token_response(raw, out);
    }
};

int main() {
    kurlyk::OAuthConfig config;
    TestClient client(config);

    // Custom parser: success
    client.set_token_parser([](const std::string& raw,
                               kurlyk::OAuthToken& token,
                               std::string& err) -> bool {
        if (raw.find("access_token") == std::string::npos) {
            err = "no access_token";
            return false;
        }
        token.access_token = "parsed_token";
        token.token_type = "Bearer";
        return true;
    });

    {
        kurlyk::AuthResult result;
        if (!client.test_parse("access_token=ok", result)) return 1;
        if (!result.success) return 1;
        if (result.token.access_token != "parsed_token") return 1;
        if (result.token.token_type != "Bearer") return 1;
    }

    // Custom parser: failure
    {
        kurlyk::AuthResult result;
        if (client.test_parse("bad", result)) return 1;
        if (result.success) return 1;
        if (result.error != kurlyk::AuthError::InvalidResponse) return 1;
        if (result.error_message != "no access_token") return 1;
    }

    // No custom parser: UnsupportedFlow
    TestClient client2(config);
    {
        kurlyk::AuthResult result;
        if (client2.test_parse("anything", result)) return 1;
        if (result.success) return 1;
        if (result.error != kurlyk::AuthError::UnsupportedFlow) return 1;
    }

    return 0;
}
