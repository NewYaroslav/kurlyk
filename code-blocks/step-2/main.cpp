#include <iostream>
#include <string>
#include <vector>

/** \brief Класс для хранения данных Cookie
 */
class Cookie {
public:
    std::string name;
    std::string value;
    std::string path;
    uint64_t expiration_date = 0;
};

inline bool case_insensitive_equal(const std::string &str1, const std::string &str2) noexcept {
    return str1.size() == str2.size() &&
       std::equal(str1.begin(), str1.end(), str2.begin(), [](char a, char b) {
         return tolower(a) == tolower(b);
       });
}

std::vector<Cookie> parse_cookie(std::string cookie) noexcept {
    std::vector<Cookie> temp;
    std::vector<std::string> list_fragment;
    cookie += ";";
    const std::string secure_prefix("__Secure-");
    const std::string host_prefix("__Host-");
    std::size_t start_pos = 0;
    while(true) {
        bool is_option = false;
        std::size_t found_separator = cookie.find_first_of("=;", start_pos);
        if(found_separator != std::string::npos) {
            std::string name = cookie.substr(start_pos, (found_separator - start_pos));

            if(name.size() > host_prefix.size() && name.substr(0,2) == "__") {

                std::size_t found_separator_prefix = name.find_first_of("-");
                if(found_separator_prefix != std::string::npos) {
                    name = name.substr(found_separator_prefix + 1, name.size() - found_separator_prefix - 1);
                    std::cout << "-3 name = " << name << std::endl;
                }
            } else {
                if (case_insensitive_equal(name, "expires") ||
                    case_insensitive_equal(name, "max-age") ||
                    case_insensitive_equal(name, "path") ||
                    case_insensitive_equal(name, "domain") ||
                    case_insensitive_equal(name, "samesite")) {
                    is_option = true;
                } else
                if (case_insensitive_equal(name, "secure") ||
                    case_insensitive_equal(name, "httponly")) {
                    if(cookie[found_separator ] == ';') start_pos = found_separator + 2;
                    else start_pos = found_separator + 1;
                    continue;
                }
            }

            start_pos = cookie.find("; ", found_separator + 1);
            if(start_pos == std::string::npos) {
                std::string value = cookie.substr(found_separator + 1, cookie.size() - found_separator - 2);
                Cookie cookie;
                cookie.name = name;
                cookie.value = value;
                if(!is_option) temp.push_back(cookie);
                break;
            }
            std::string value = cookie.substr(found_separator + 1, start_pos - found_separator - 1);
            start_pos += 2;
            Cookie cookie;
            cookie.name = name;
            cookie.value = value;
            if(!is_option) temp.push_back(cookie);
        } else {
            break;
        }
    }
    return std::move(temp);
}

int main() {
    std::string cookie = "__cfduid=da5ee5af958161938647088c7e6b659001616206848; expires=Mon, 19-Apr-21 02:20:48 GMT; path=/; domain=.primexbt.com; HttpOnly; SameSite=Lax; Secure";
    std::vector<Cookie> r1 = parse_cookie(cookie);
    for(size_t i = 0; i < r1.size(); ++i) {
        std::cout << r1[i].name << "=" << r1[i].value << std::endl;
    }

    std::string cookie2 = "__Secure-__cfduid=da5ee5af958161938647088c7e6b659001616206848; expires=Mon, 19-Apr-21 02:20:48 GMT; path=/; domain=.primexbt.com; HttpOnly; SameSite=Lax; Secure";
    std::vector<Cookie> r2 = parse_cookie(cookie2);
    for(size_t i = 0; i < r2.size(); ++i) {
        std::cout << r2[i].name << "=" << r2[i].value << std::endl;
    }

    std::string cookie3 = "bws=12345; __cfduid=da5ee5af958161938647088c7e6b659001616206848; expires=Mon, 19-Apr-21 02:20:48 GMT; path=/; domain=.primexbt.com; HttpOnly; SameSite=Lax; Secure; cop=bmkdsewe";
    std::vector<Cookie> r3 = parse_cookie(cookie3);
    for(size_t i = 0; i < r3.size(); ++i) {
        std::cout << r3[i].name << "=" << r3[i].value << std::endl;
    }
    return 0;
}
