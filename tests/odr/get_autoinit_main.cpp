#include <iostream>
#include <kurlyk.hpp>

extern "C" kurlyk::startup::AutoInitializer* get_auto_init_a();
extern "C" kurlyk::startup::AutoInitializer* get_auto_init_b();

int main() {

    auto* a = get_auto_init_a();
    auto* b = get_auto_init_b();

    std::cout << "AutoInitializer A: " << (const void*)a
              << " AutoInitializer B: " << (const void*)b << "\n";

    if (a != b) {
        std::cerr << "There are 2 different AutoInitializer instances!\n";
        return 1;
    }
	
	std::cout << "There's only AutoInitializer, singlton works correctly" << std::endl;
	
    return 0;
}
