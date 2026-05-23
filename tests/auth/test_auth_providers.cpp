#include <kurlyk.hpp>
#include <string>

int main() {
    // BearerTokenAuthProvider header injection
    {
        kurlyk::Headers headers;
        kurlyk::http::auth::BearerTokenAuthProvider bearer("abc123");
        if (!bearer.authorize(headers)) return 1;
        auto it = headers.find("Authorization");
        if (it == headers.end()) return 1;
        if (it->second != "Bearer abc123") return 1;
    }

    // BearerTokenAuthProvider HttpRequest injection
    {
        kurlyk::HttpRequest req;
        kurlyk::http::auth::BearerTokenAuthProvider bearer("xyz789");
        if (!bearer.authorize(req)) return 1;
        auto it = req.headers.find("Authorization");
        if (it == req.headers.end()) return 1;
        if (it->second != "Bearer xyz789") return 1;
    }

    // ApiKeyAuthProvider HEADER placement
    {
        kurlyk::Headers headers;
        kurlyk::http::auth::ApiKeyAuthProvider api_key(
            "X-Api-Key", "secret", kurlyk::http::auth::ApiKeyPlacement::HEADER);
        if (!api_key.authorize(headers)) return 1;
        auto it = headers.find("X-Api-Key");
        if (it == headers.end()) return 1;
        if (it->second != "secret") return 1;
    }

    // ApiKeyAuthProvider QUERY placement on HttpRequest
    {
        kurlyk::HttpRequest req;
        req.url = "https://example.com/api";
        kurlyk::http::auth::ApiKeyAuthProvider api_key(
            "key", "val", kurlyk::http::auth::ApiKeyPlacement::QUERY);
        if (!api_key.authorize(req)) return 1;
        if (req.url != "https://example.com/api?key=val") return 1;
    }

    // ApiKeyAuthProvider QUERY placement with existing query
    {
        kurlyk::HttpRequest req;
        req.url = "https://example.com/api?foo=bar";
        kurlyk::http::auth::ApiKeyAuthProvider api_key(
            "key", "val", kurlyk::http::auth::ApiKeyPlacement::QUERY);
        if (!api_key.authorize(req)) return 1;
        if (req.url != "https://example.com/api?foo=bar&key=val") return 1;
    }

    return 0;
}
