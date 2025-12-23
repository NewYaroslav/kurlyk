#include <iostream>
#include <kurlyk.hpp>

extern "C" kurlyk::core::NetworkWorker* get_net_worker_a();
extern "C" kurlyk::core::NetworkWorker* get_net_worker_b();

extern "C" kurlyk::HttpRequestManager* get_http_request_manager_a();
extern "C" kurlyk::HttpRequestManager* get_http_request_manager_b();

extern "C" kurlyk::WebSocketManager* get_websocket_manager_a();
extern "C" kurlyk::WebSocketManager* get_websocket_manager_b();

int main() {
	
	// Networker
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
	
	// HttpRequestManager
	auto* HttpManager_a = get_http_request_manager_a();
	auto* HttpManager_b = get_http_request_manager_b();
	
	std::cout 	<< "HttpRequestManager A address: " << static_cast<const void*>(HttpManager_a) 
				<< "	HttpRequestManager B address: " << static_cast<const void*>(HttpManager_b) 
				<< std::endl;
				
	if (HttpManager_a != HttpManager_b) {
		std::cout << "There are 2 different HttpRequestManager instances!" << std::endl;
		return 1;
	}
	
	std::cout << "There's only HttpRequestManager, singlton works correctly" << std::endl;
	
	//WebSocketManager
	auto* WsManager_a = get_websocket_manager_a();
	auto* WsManager_b = get_websocket_manager_b();
	
	std::cout 	<< "WebSocketManager A address: " << static_cast<const void*>(WsManager_a) 
				<< "	WebSocketManager B address: " << static_cast<const void*>(WsManager_b) 
				<< std::endl;
				
	if (WsManager_a != WsManager_b) {
		std::cout << "There are 2 different WebSocketManager instances!" << std::endl;
		return 1;
	}
	
	std::cout << "There's only WebSocketManager, singlton works correctly" << std::endl;
	
	return 0;
}
