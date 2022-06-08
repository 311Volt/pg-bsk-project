
#ifndef SRC_APP_APPSTATE
#define SRC_APP_APPSTATE

#include <exception>
#include <stdexcept>
#include <future>
#include <vector>
#include <string>
#include <array>
#include <queue>
#include <mutex>

#include "../crypto/Crypto.hpp"

#include <rpc/client.h>
#include <rpc/server.h>
#include <rpc/rpc_error.h>
#include <rpc/msgpack/v1/object.hpp>

#include <fmt/format.h>


#include "Codes.hpp"
#include "FuturePromise.hpp"
#include "Filestate.hpp"


#define DEBUG() {printf(" DEBUG: %s:%i\n", __FILE__, __LINE__); fflush(stdout);}

using Array12 = std::array<uint8_t, 12>;
using Array16 = std::array<uint8_t, 16>;
using Array32 = std::array<uint8_t, 32>;
using Array33 = std::array<uint8_t, 33>;
using Array64 = std::array<uint8_t, 64>;

using EcPublicKey = std::array<uint8_t, ec::PUBLIC_KEY_SIZE>;
using EcPrivateKey = std::array<uint8_t, ec::PRIVATE_KEY_SIZE>;
using EcSignature = std::array<uint8_t, ec::SIGNATURE_SIZE>;
using ChachaNonce = std::array<uint8_t, chacha::NONCE_SIZE>;

class KexError: public std::runtime_error{using std::runtime_error::runtime_error;};
class KexKeygenFailed: public KexError{using KexError::KexError;};
class KexSignFailed: public KexError{using KexError::KexError;};
class KexConnectionFailed: public KexError{using KexError::KexError;};
class SendEncryptedPacketFailed: public std::runtime_error{using std::runtime_error::runtime_error;};
class SendMessageFailed: public std::runtime_error{using std::runtime_error::runtime_error;};

struct KexMessage;

struct Message {
	MSG_TYPE msg_type;
	ENCRYPTION_MODE cipher_variant;
	ChachaNonce nonce;
	std::vector<uint8_t> encrypted_data;

	MSGPACK_DEFINE_ARRAY(msg_type, cipher_variant, nonce, encrypted_data);
};

class AppState {
public:
	static AppState* singleton;
	
	AppState(std::string myIp, int32_t port);
	~AppState();
	
	ERROR_CODE ConnectAndHandshake(std::string ip, int32_t port);
	
	void BindAll();
	
	void GenerateKey();
	void LoadKey(std::string keyFilePath, const std::string& passphrase);
	void SaveKey(std::string keyFilePath, const std::string& passphrase);
	
	KexMessage ReceiveKex(KexMessage kex);
	
	
	template<typename Ret>
	Future<Ret> SendEncryptedPacket(std::string functionName,
			MSG_TYPE msgType, void* data, uint32_t bytes);
	
	
	Future<uint32_t> SendMessage(std::string message);
	uint32_t ReceiveMessage(Message message);

	void PushMessage(const std::string& message);
	bool PopMessage(std::string& message);
	
	std::shared_ptr<Filestate> SendFile(std::string fileName);
	uint32_t ReceiveFileMeta(Message message);
	int32_t ReceiveFileBlock(Message message);
	
public:
	
	void EncryptMessage(MSG_TYPE type, const void* plaintext, size_t length,
			Message& message);
	bool DecryptMessage(const Message& message,
			std::vector<uint8_t>& plaintext);
	
	void Encrypt(const void* plaintext, size_t plaintextLength,
			ChachaNonce& nonce, std::vector<uint8_t>& ciphertext,
			const void* ad, size_t adLength, ENCRYPTION_MODE encryptionMode);
	bool Decrypt(const void* ciphertext, size_t ciphertextLength,
			const ChachaNonce& nonce, std::vector<uint8_t>& plaintext,
			const void* ad, size_t adLength, ENCRYPTION_MODE encryptionMode);
	
public:
	
	ENCRYPTION_MODE currentEncryptionMode;
	
	rpc::server rpcServer;
	std::unique_ptr<rpc::client> client;
	
	EcPublicKey publicKey;
	EcPrivateKey privateKey;
	Array32 sharedKey;
	EcPublicKey theirPublicKey;
	
	std::shared_ptr<Filestate> filestate;
	
	std::queue<std::string> receivedMessages;
	std::mutex mutex;
	std::string ipAddress;
	int32_t port;

	std::string theirIPAddress;
	int32_t theirPort;
};




template<typename Ret>
Future<Ret> AppState::SendEncryptedPacket(std::string functionName,
		MSG_TYPE msgType, void* data, uint32_t bytes) {
	if(client == NULL) {
		throw SendEncryptedPacketFailed(fmt::format("Failed to send encrypted packet due to NULL value of AppState::client"));
	}
	Message msg;
	EncryptMessage(msgType, data, bytes, msg);
	
	
	std::shared_future<clmdep_msgpack::v1::object_handle> future =
		client->async_call(functionName, msg).share();
	
	return std::async([](std::shared_future<clmdep_msgpack::v1::object_handle> arg)->Ret {
			auto& a = arg.get();
			return ((clmdep_msgpack::v1::object_handle*)&a)->as<Ret>();
		}, future).share();
}

#endif /* SRC_APP_APPSTATE */

