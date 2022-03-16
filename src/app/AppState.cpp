
#include <functional>

#include "AppState.hpp"
#include "crypto/Crypto.hpp"

AppState::AppState(std::string myIp, int32_t port) : rpcServer(port),
	ipAddress(myIp), port(port) {
		client = NULL;
		GenerateKey();
		BindAll();
		rpcServer.async_run(1);
		currentEncryptionMode = ENCRYPTION_MODE::CHACHA20_POLY1305;
}

AppState::~AppState() {
	if(client)
		delete client;
}

void AppState::BindAll() {
	rpcServer.bind("Message", std::bind(&AppState::ReceiveMessage, this,
				std::placeholders::_1, std::placeholders::_2,
				std::placeholders::_3, std::placeholders::_4));
	rpcServer.bind("Kex", std::bind(&AppState::ReceiveKex, this,
				std::placeholders::_1, std::placeholders::_2,
				std::placeholders::_3, std::placeholders::_4,
				std::placeholders::_5, std::placeholders::_6));
}

bool AppState::ConnectAndHandshake(std::string ip, int32_t port) {
	client = new rpc::client(ip, port);
	
	EcPublicKey ecdhePubkey;
	EcPrivateKey privateEcdheKey;
	ec::GenKey(privateEcdheKey.data(), ecdhePubkey.data());
	
	EcSignature signature;
	
	Array32 hash;
	digest::sha256()
		.absorb((uint32_t)KEX_INIT)
		.absorb(this->ipAddress.c_str(), this->ipAddress.size()+1)
		.absorb(this->port)
		.absorb(publicKey.data(), publicKey.size())
		.absorb(ecdhePubkey.data(), ecdhePubkey.size())
		.finalize(hash.data());
	ec::Sign(privateKey.data(), hash.data(), signature.data());
	
	auto response = client->call("Kex", KEX_INIT, publicKey, ecdhePubkey,
			this->ipAddress, this->port, signature);
	
	// TODO: finalize kex: check for valid return
	
	
}

AppState::KexResponse AppState::ReceiveKex(uint32_t msgType,
		const EcPublicKey& other_pubkey,
		const EcPublicKey& ecdhe_pubkey,
		std::string ipAddress, int32_t port,
		const EcSignature& message_signature) {
	
}

void AppState::SendMessage(std::string message) {
	
	
}

uint32_t AppState::ReceiveMessage(uint32_t msgType,
		uint32_t encryptionMode,
		const ChachaNonce& nonce,
		const std::vector<uint8_t>& encryptedMessage) {
	std::string message;
	if(encryptionMode == ENCRYPTION_MODE::CHACHA20) {
		
	} else {
		Array32 hash;
		digest::sha256()
			.absorb(msgType)
			.absorb(encryptionMode)
			.finalize(hash.data());
		
		message.resize(encryptedMessage.size()-chacha::MAC_SIZE+1);
		
		bool res = chacha::decrypt(sharedKey.data(), nonce.data(),
				encryptedMessage.data(), (uint8_t*)&(message[0]),
				encryptedMessage.size(), hash.data(), hash.size());
		if(!res)
			return ERROR_CODE::FAILED_VALIDATION_MESSAGE;
	}
	PushReceivedMessage(message);
	return ERROR_CODE::SUCCESS;
}

