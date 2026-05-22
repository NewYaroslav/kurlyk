#include <kurlyk/utils/Pkce.hpp>
#include <string>
#include <set>
#include <cctype>

int main() {
    // Verifier length must be within 43..128
    for (std::size_t len : {43, 64, 100, 128}) {
        std::string verifier = kurlyk::utils::generate_code_verifier(len);
        if (verifier.size() < 43 || verifier.size() > 128) return 1;

        // Allowed charset
        std::set<char> allowed;
        const char* allowed_str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
        for (const char* p = allowed_str; *p; ++p) allowed.insert(*p);
        for (char c : verifier) {
            if (!allowed.count(c)) return 1;
        }
    }

    // Default length (64)
    std::string verifier = kurlyk::utils::generate_code_verifier();
    if (verifier.size() != 64) return 1;

    // Challenge differs from verifier
    std::string challenge = kurlyk::utils::make_s256_code_challenge(verifier);
    if (challenge == verifier) return 1;
    if (challenge.empty()) return 1;
    if (challenge.find('=') != std::string::npos) return 1;

    // make_pkce_pair consistency
    kurlyk::utils::PkcePair pair = kurlyk::utils::make_pkce_pair();
    if (pair.code_verifier.empty()) return 1;
    if (pair.code_challenge.empty()) return 1;
    if (pair.code_challenge_method != "S256") return 1;

    return 0;
}
