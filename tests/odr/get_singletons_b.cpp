#include <kurlyk.hpp>
#include <memory>

extern "C" kurlyk::core::NetworkWorker* get_net_worker_b() { 
	return std::addressof(kurlyk::core::NetworkWorker::get_instance()); 
}

extern "C" kurlyk::HttpRequestManager* get_http_request_manager_b() { 
	return std::addressof(kurlyk::HttpRequestManager::get_instance()); 
}

extern "C" kurlyk::WebSocketManager* get_websocket_manager_b() { 
	return std::addressof(kurlyk::WebSocketManager::get_instance()); 
}