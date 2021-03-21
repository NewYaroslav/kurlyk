#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

/** \brief Функция сравнения строк для CaseInsensitiveMultimap
 *
 * Оригинал тут: https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/master/utility.hpp#L42
 * Спасибо Ole Christian Eidheim за библиотеку!
 */
inline bool case_insensitive_equal(const std::string &str1, const std::string &str2) noexcept {
    return str1.size() == str2.size() &&
       std::equal(str1.begin(), str1.end(), str2.begin(), [](char a, char b) {
         return tolower(a) == tolower(b);
       });
}

/** \brief Оператор сравнения строк для CaseInsensitiveMultimap
 *
 * Оригинал тут: https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/master/utility.hpp#L48
 * Спасибо Ole Christian Eidheim за библиотеку!
 */
class CaseInsensitiveEqual {
public:
    bool operator()(const std::string &str1, const std::string &str2) const noexcept {
        return case_insensitive_equal(str1, str2);
    }
};

// Based on https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226

/** \brief Hash функция для CaseInsensitiveMultimap
 *
 * Основано на: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226
 * Оригинал тут: https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/master/utility.hpp#L55
 * Спасибо Ole Christian Eidheim за библиотеку!
 */
class CaseInsensitiveHash {
public:
    std::size_t operator()(const std::string &str) const noexcept {
        std::size_t h = 0;
        std::hash<int> hash;
        for(auto c : str)
            h ^= hash(tolower(c)) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

/** \brief Класс для хранения данных Cookie
 */
class Cookie {
public:
    std::string name;
    std::string value;
    std::string path;
    uint64_t expiration_date = 0;
};

using CaseInsensitiveMultimap = std::unordered_multimap<std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEqual>;
using CaseInsensitiveMultimap2 = std::unordered_multimap<std::string, Cookie, CaseInsensitiveHash, CaseInsensitiveEqual>;

int main() {
    CaseInsensitiveMultimap test;
    test.emplace("BWS", "12345");
    test.emplace("bws", "6789A");
    test.emplace("bws", "6789B");
    auto it = test.find("bws");
    if(it != test.end()) {
        std::cout << it->second << std::endl;
    }
    auto it2 = test.find("BWS");
    if(it2 != test.end()) {
        std::cout << it->second << std::endl;
    }

    CaseInsensitiveMultimap test2;
    test2.emplace("BWS", "BGHFS-44");
    test2.emplace("ABC", "bnmf33");
    test.insert(test2.begin(), test2.end());

    auto it3 = test.find("BWS");
    if(it3 != test.end()) {
        std::cout << it3->second << std::endl;
    }

    auto it4 = test.find("ABC");
    if(it4 != test.end()) {
        std::cout << it4->second << std::endl;
    }

    CaseInsensitiveMultimap2 test3;
    Cookie cookie;
    cookie.value = "12345";
    test3.emplace("BWS", cookie);

    auto it5 = test3.find("bws");
    if(it5 != test3.end()) {
        std::cout << it5->second.value << std::endl;
    }
    return 0;
}
