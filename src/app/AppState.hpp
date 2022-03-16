
#ifndef PG_BSK_PROJECT_APP_STATE_HPP
#define PG_BSK_PROJECT_APP_STATE_HPP

#include <vector>
#include <string>

#include <crypto/Crypto.hpp>

#include <rpc/client.h>
#include <rpc/server.h>

class AppState {
public:
	
	static AppState* singleton;
	
	AppState(int port);
	~AppState();
	
public:
	
	rpc::server rpcServer;
	rpc::client* client;
	
	
	
	std::vector<uint8_t> privateKey, publicKey, sharedKey;
	std::vector<uint8_t> ecdhePublicKey;
	std::vector<std::string> receivedMessages;
	std::string myIp;
};

#endif

