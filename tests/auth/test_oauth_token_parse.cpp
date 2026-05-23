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

    // Custom parser: simulates error response with invalid_grant
    {
        TestClient client3(config);
        client3.set_token_parser([](const std::string& raw,
                                     kurlyk::OAuthToken& token,
                                     std::string& err) -> bool {
            if (raw.find("invalid_grant") != std::string::npos) {
                err = "invalid_grant";
                return false;
            }
            token.access_token = "ok";
            token.token_type = "Bearer";
            return true;
        });
        kurlyk::AuthResult result;
        if (client3.test_parse("invalid_grant", result)) return 1;
        if (result.success) return 1;
        if (result.error != kurlyk::AuthError::InvalidResponse) return 1;
        if (result.error_message != "invalid_grant") return 1;
    }

    // validate_state: empty strings return false
    {
        kurlyk::http::auth::OAuthPkceClient empty_state(config);
        if (empty_state.validate_state("")) return 1;
    }

    // validate_state: matching state returns true
    {
        kurlyk::OAuthConfig good_config;
        good_config.client_id = "client";
        good_config.authorization_endpoint = "https://example.com/auth";
        good_config.redirect_uri = "https://example.com/cb";
        kurlyk::http::auth::OAuthPkceClient match_client(good_config);
        std::string url = match_client.build_authorization_url();
        (void)url;
        if (!match_client.validate_state(match_client.state())) return 1;
    }

    // validate_state: mismatching state returns false
    {
        kurlyk::OAuthConfig good_config;
        good_config.client_id = "client";
        good_config.authorization_endpoint = "https://example.com/auth";
        good_config.redirect_uri = "https://example.com/cb";
        kurlyk::http::auth::OAuthPkceClient mismatch_client(good_config);
        std::string url = mismatch_client.build_authorization_url();
        (void)url;
        if (mismatch_client.validate_state("wrong_state")) return 1;
    }

    return 0;
}
