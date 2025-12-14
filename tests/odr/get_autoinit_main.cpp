#include <iostream>
#include <kurlyk.hpp>

extern "C" kurlyk::startup::AutoInitializer* get_auto_init_a();
extern "C" kurlyk::startup::AutoInitializer* get_auto_init_b();

int main() {
	std::cout << "main start\n";
	
	#ifdef KURLYK_AUTO_INIT
	  std::cout << "KURLYK_AUTO_INIT defined\n";
	#endif
	#if defined(KURLYK_AUTO_INIT)
	  std::cout << "KURLYK_AUTO_INIT=" << KURLYK_AUTO_INIT << "\n";
	#endif

    auto* a = get_auto_init_a();
    auto* b = get_auto_init_b();

    std::cout << "AutoInitializer A: " << (const void*)a
              << " AutoInitializer B: " << (const void*)b << "\n";

    if (a != b) {
        std::cerr << "There are 2 different AutoInitializer instances!\n";
        return 1;
    }
	
	std::cout << "There's only AutoInitializer, singlton works correctly" << std::endl;
	
	std::cout << "before return\n";
    return 0;
}
