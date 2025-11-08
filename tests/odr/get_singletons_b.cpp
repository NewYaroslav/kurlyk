#include <kurlyk/core/NetworkWorker.hpp>
#include <memory>

extern "C" kurlyk::core::NetworkWorker* get_net_worker_b() { 
	return std::addressof(kurlyk::core::NetworkWorker::get_instance()); 
}