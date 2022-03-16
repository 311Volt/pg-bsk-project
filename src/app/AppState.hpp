
#ifndef PG_BSK_PROJECT_APP_STATE_HPP
#define PG_BSK_PROJECT_APP_STATE_HPP

#include <vector>
#include <string>
#include <array>

#include "../crypto/Crypto.hpp"

#include <rpc/client.h>
#include <rpc/server.h>

#include "Codes.hpp"

class AppState {
public:
	
	using Array12 = std::array<uint8_t, 12>;
	using Array16 = std::array<uint8_t, 16>;
	using Array32 = std::array<uint8_t, 32>;
	using Array33 = std::array<uint8_t, 33>;
	using Array64 = std::array<uint8_t, 64>;
	
	using EcPublicKey = std::array<uint8_t, ec::PUBLIC_KEY_SIZE>;
	using EcPrivateKey = std::array<uint8_t, ec::PRIVATE_KEY_SIZE>;
	using EcSignature = std::array<uint8_t, ec::SIGNATURE_SIZE>;
	using ChachaNonce = std::array<uint8_t, chacha::NONCE_SIZE>;
	
	static AppState* singleton;
	
	AppState(std::string myIp, int32_t port);
	~AppState();
	
	bool ConnectAndHandshake(std::string ip, int32_t port);
	
	void BindAll();
	
	
	void GenerateKey();
	void LoadKey(std::string keyFilePath, const std::string& passphrase);
	void SaveKey(std::string keyFilePath, const std::string& passphrase);
	
	
	
	
	struct KexResponse {
		std::array<uint8_t, ec::PUBLIC_KEY_SIZE> publicKey;
		std::array<uint8_t, ec::PUBLIC_KEY_SIZE> publicEcdheKey;
		std::array<uint8_t, ec::HASH_TO_SIGN_SIZE> messageSignature;
		
		MSGPACK_DEFINE_ARRAY(publicKey, publicEcdheKey, messageSignature);
	};
	
	KexResponse ReceiveKex(uint32_t msgType,
			const EcPublicKey& other_pubkey,
			const EcPublicKey& ecdhe_pubkey,
			std::string ipAddress, int32_t port,
			const EcSignature& message_signature);
	
	
	
	
	
	
	void SendMessage(std::string message);
	
	uint32_t ReceiveMessage(uint32_t msgType, uint32_t encryptionMode,
			const ChachaNonce& nonce,
			const std::vector<uint8_t>& message);
	
	void PushReceivedMessage(std::string message);
	
public:
	
	ENCRYPTION_MODE currentEncryptionMode;
	
	rpc::server rpcServer;
	rpc::client* client;
	
	EcPublicKey publicKey;
	EcPrivateKey privateKey;
	Array32 sharedKey;
	
	std::vector<std::string> receivedMessages;
	std::string ipAddress;
	int32_t port;
};

#endif

