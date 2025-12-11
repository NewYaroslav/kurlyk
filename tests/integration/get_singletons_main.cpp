#include <iostream>
#include <kurlyk.hpp>

extern "C" kurlyk::core::NetworkWorker* get_net_worker_a();
extern "C" kurlyk::core::NetworkWorker* get_net_worker_b();

int main() {
	auto* NetWork_a = get_net_worker_a();
	auto* NetWork_b = get_net_worker_b();
	
	std::cout 	<< "NetworkWorker A address: " << static_cast<const void*>(NetWork_a) 
				<< "	NetworkWorker B address: " << static_cast<const void*>(NetWork_b) 
				<< std::endl;
				
	if (NetWork_a != NetWork_b) {
		std::cout << "There are 2 different NetworkWorker instances!" << std::endl;
		return 1;
	}
	
	std::cout << "There's only NetworkWorker, singlton works correctly" << std::endl;
	return 0;
}
