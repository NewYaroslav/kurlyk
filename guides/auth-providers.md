# Authentication Providers

`kurlyk` ships with two built-in `IAuthProvider` implementations for attaching credentials to outgoing HTTP requests. Providers are stateless and modify either `HttpRequest` or `Headers` in-place.

## BearerTokenAuthProvider

Injects an `Authorization: Bearer <token>` header, replacing any previous `Authorization` value.

```cpp
kurlyk::http::auth::BearerTokenAuthProvider auth("my_api_key");

kurlyk::HttpRequest request;
request.url = "https://api.example.com/data";
auth.authorize(request);           // adds Authorization to request.headers

kurlyk::Headers headers;
auth.authorize(headers);           // adds Authorization to the header map
```

## ApiKeyAuthProvider

Injects an API key either as a custom HTTP header or as a URL query parameter.

```cpp
// As a header
kurlyk::http::auth::ApiKeyAuthProvider header_auth(
    "X-Api-Key", "secret", kurlyk::http::auth::ApiKeyPlacement::HEADER);

kurlyk::Headers headers;
header_auth.authorize(headers);
// headers now contains X-Api-Key: secret
```

```cpp
// As a query parameter
kurlyk::http::auth::ApiKeyAuthProvider query_auth(
    "key", "myvalue", kurlyk::http::auth::ApiKeyPlacement::QUERY);

kurlyk::HttpRequest request;
request.url = "https://api.example.com/data";
query_auth.authorize(request);
// request.url is now "https://api.example.com/data?key=myvalue"
```

Query parameters are percent-encoded automatically via `kurlyk::utils::percent_encode`.

## Implementing a custom provider

Derive from `IAuthProvider` and implement `authorize(HttpRequest&)` and `authorize(Headers&)`.

```cpp
class MyAuthProvider : public kurlyk::http::auth::IAuthProvider {
public:
    bool authorize(kurlyk::HttpRequest& request) const override {
        request.headers.emplace("X-Special-Auth", compute_signature(request));
        return true;
    }

    bool authorize(kurlyk::Headers& headers) const override {
        headers.emplace("X-Special-Auth", "static-signature");
        return true;
    }
};
```

## TokenStorage interface

For persisting OAuth tokens, implement `ITokenStorage`:

```cpp
class FileTokenStorage : public kurlyk::http::auth::ITokenStorage {
public:
    bool save(const kurlyk::OAuthToken& token) override { /* ... */ }
    bool load(kurlyk::OAuthToken& out) override { /* ... */ }
    bool clear() override { /* ... */ }
};
```

The interface does not prescribe encryption — that is the responsibility of the concrete implementation.

## See also

- `examples/bearer_token_auth_provider_example.cpp`
- `examples/api_key_auth_provider_example.cpp`
- `include/kurlyk/http/auth/BearerTokenAuthProvider.hpp`
- `include/kurlyk/http/auth/ApiKeyAuthProvider.hpp`
- `include/kurlyk/http/auth/TokenStorage.hpp`
