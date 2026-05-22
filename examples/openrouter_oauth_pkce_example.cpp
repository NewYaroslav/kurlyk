#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <iostream>

int main() {
    kurlyk::init(true);
    try {
        kurlyk::OAuthConfig config;
        config.client_id = "your_openrouter_client_id";
        config.authorization_endpoint = "https://openrouter.ai/oauth/authorize";
        config.token_endpoint = "https://openrouter.ai/oauth/token";
        config.redirect_uri = "http://127.0.0.1:8080/callback";
        config.scope = "read";
        config.use_pkce = true;

        kurlyk::http::auth::OAuthPkceClient oauth(config);

        std::string auth_url = oauth.build_authorization_url();
        KURLYK_PRINT << "Open this URL in a browser:" << std::endl;
        KURLYK_PRINT << auth_url << std::endl;
        KURLYK_PRINT << "State: " << oauth.state() << std::endl;

        // In a real application, after the user is redirected back,
        // extract the "code" and "state" from the redirect URL and call exchange_code:
        //
        // std::string code = receive_code_from_redirect();
        // std::string returned_state = receive_state_from_redirect();
        //
        // if (!oauth.validate_state(returned_state)) {
        //     KURLYK_PRINT << "State mismatch!" << std::endl;
        //     return 1;
        // }
        //
        // kurlyk::AuthResult result;
        // if (oauth.exchange_code(code, result)) {
        //     KURLYK_PRINT << "Access token: " << result.token.access_token << std::endl;
        // } else {
        //     KURLYK_PRINT << "Error: " << result.error_message << std::endl;
        // }

    } catch (const std::exception& e) {
        KURLYK_PRINT << "Error: " << e.what() << std::endl;
    }

    kurlyk::deinit();
    return 0;
}
