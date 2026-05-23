# OAuth2 / PKCE Authentication

`kurlyk` provides a lightweight, standalone OAuth2 Authorization Code + PKCE client that works on top of the existing HTTP helpers. It does not launch a local redirect server, open a browser, or persist tokens encrypted — those are the caller's responsibility.

## What is included

- `OAuthPkceClient` — builds authorization URLs, exchanges codes for tokens, refreshes access tokens, and validates `state`.
- `OAuthConfig` — client id/secret, endpoints, redirect URI, scope, PKCE toggle, and optional parameters (`audience`, `prompt`, `access_type`).
- `OAuthToken` — `access_token`, `refresh_token`, `token_type`, `scope`, `expires_at_ms`, `raw_response`.
- `AuthResult` — `success`, `AuthError` code, `error_message`, token, and raw server response.

## What is NOT included (MVP)

- Automatic local redirect server (you receive the code/state from your own HTTP handler).
- Token auto-refresh or background renewal.
- Encrypted token storage (implement `ITokenStorage` if you need it).
- OpenID Connect `id_token` validation, JWT service accounts, device code, or client-credentials flow.
- Browser auto-open or platform credential vaults.

## Quick flow

```cpp
#include <kurlyk.hpp>
#include <iostream>

int main() {
    kurlyk::OAuthConfig config;
    config.client_id              = "your_client_id";
    config.authorization_endpoint = "https://provider.com/oauth/authorize";
    config.token_endpoint         = "https://provider.com/oauth/token";
    config.redirect_uri           = "http://127.0.0.1:8080/callback";
    config.scope                  = "read write";
    config.use_pkce               = true;

    kurlyk::http::auth::OAuthPkceClient oauth(config);

    std::string url = oauth.build_authorization_url();
    std::cout << "Open in browser:\n" << url << "\n";
    std::cout << "State: " << oauth.state() << "\n";

    // After the user approves and is redirected back, extract `code` and `state`.
    // Then:
    //
    // if (!oauth.validate_state(returned_state)) { ... }
    //
    // kurlyk::AuthResult result;
    // if (oauth.exchange_code(auth_code, result)) {
    //     std::cout << "Access token: " << result.token.access_token << "\n";
    // } else {
    //     std::cerr << "Error: " << result.error_message << "\n";
    // }

    return 0;
}
```

## Token refresh

```cpp
kurlyk::AuthResult result;
if (oauth.refresh_access_token(result.token.refresh_token, result)) {
    std::cout << "New token: " << result.token.access_token << "\n";
}
```

## Custom token parser (non-JSON builds)

If `KURLYK_JSON_SUPPORT` is `0`, `OAuthPkceClient` cannot parse JSON by default. Provide a custom parser:

```cpp
oauth.set_token_parser([](const std::string& raw,
                          kurlyk::OAuthToken& out,
                          std::string& err) -> bool {
    // Parse raw string into out; return false and set err on failure.
    return true;
});
```

## See also

- `examples/openrouter_oauth_pkce_example.cpp`
- `include/kurlyk/http/auth/OAuthPkceClient.hpp`
