#define KURLYK_AUTO_INIT 0

#include <iostream>
#include <kurlyk.hpp>

int main() {
    kurlyk::init(false);
    kurlyk::process();
    kurlyk::deinit();
    kurlyk::deinit();

    std::cout << "Manual synchronous lifecycle deinit completed" << std::endl;
    return 0;
}
