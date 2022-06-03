
#include <cmath>
#include <functional>
#include <memory>

#include "AppState.hpp"
#include "crypto/Crypto.hpp"

#include <fmt/format.h>

AppState* AppState::singleton = NULL;

void KexMessage::GenerateDigest(Array32& hash) {
	digest::sha256()
		.absorb((uint8_t)error_code)
		.absorb(publicKey.data(), publicKey.size())
		.absorb(publicEcdheKey.data(), publicEcdheKey.size())
		.absorb(ipaddress.data(), ipaddress.size())
		.absorb(port)
		.finalize(hash.data());
}

bool KexMessage::Verify() {
	Array32 hash;
	GenerateDigest(hash);
	return ec::Verify(publicKey.data(), hash.data(), signature.data());
}

bool KexMessage::Sign(EcPrivateKey& privkey) {
	Array32 hash;
	GenerateDigest(hash);
	return ec::Sign(privkey.data(), hash.data(), signature.data());
}

AppState::AppState(std::string myIp, int32_t port) : rpcServer(port),
	ipAddress(myIp), port(port) {
		client = NULL;
		GenerateKey();
		BindAll();
		rpcServer.async_run(1);
		currentEncryptionMode = ENCRYPTION_MODE::CHACHA20_POLY1305;
		singleton = this;
}

AppState::~AppState() {
	if(client)
		delete client;
	if(singleton == this)
		singleton = NULL;
}

void AppState::BindAll() {
	rpcServer.bind("Message", [](Message msg)->uint32_t{
					return singleton->ReceiveMessage(msg);
	});
	rpcServer.bind("Kex", [](KexMessage msg)->KexMessage{
					return singleton->ReceiveKex(msg);
	});
	rpcServer.bind("FileMeta", [](Message msg)->uint32_t{
					return singleton->ReceiveFileMeta(msg);
	});
	rpcServer.bind("FileBlock", [](Message msg)->int32_t{
					return singleton->ReceiveFileBlock(msg);
	});
}

void AppState::GenerateKey() {
	ec::GenKey(this->privateKey.data(), this->publicKey.data());
}

ERROR_CODE AppState::ConnectAndHandshake(std::string ip, int32_t port) {
	auto newClient = std::make_unique<rpc::client>(ip, port);
	
	KexMessage kex;
	EcPrivateKey privateEcdheKey;

	if(ec::GenKey(privateEcdheKey.data(), kex.publicEcdheKey.data()) == false) {
		throw KexKeygenFailed("Key generation failed");
	}

	kex.publicKey = publicKey;
	kex.error_code = SUCCESS;
	kex.ipaddress = ipAddress;
	kex.port = this->port;

	if(kex.Sign(this->privateKey) == false) {
		throw KexSignFailed("Cannot sign private key");
	}

	KexMessage kexResponse;
	try {
		kexResponse = newClient->async_call("Kex", kex).get().as<KexMessage>();
	} catch(rpc::rpc_error& err) {
		throw KexConnectionFailed(fmt::format("Cannot connect: {}", err.what()));
	}
	
	if(kexResponse.error_code == SUCCESS) {
		if(kexResponse.Verify()) {
			if(ec::Ecdh(privateEcdheKey.data(),
						kexResponse.publicEcdheKey.data(), sharedKey.data())) {
				theirPublicKey = kexResponse.publicEcdheKey;
			} else {
				throw KexError("Key exchange failed");
			}
		} else {
			throw KexError("Response verification failed");
		}
	}

	client = newClient.release();
	return SUCCESS;
}

KexMessage AppState::ReceiveKex(KexMessage kexReceived) {
	if(kexReceived.Verify() == false) {
		KexMessage ret;
		ret.error_code = FAILED_VALIDATION_KEX;
		return ret;
	}
	
	theirPublicKey = kexReceived.publicEcdheKey;
	
	KexMessage kex;
	if(ec::Ecdhe(kexReceived.publicEcdheKey.data(), kex.publicEcdheKey.data(),
				sharedKey.data()) == false) {
		KexMessage ret;
		ret.error_code = FAILED;
		return ret;
	}
	kex.publicKey = publicKey;
	kex.error_code = SUCCESS;
	kex.ipaddress = ipAddress;
	kex.port = this->port;

	if(kex.Sign(this->privateKey) == false) {
		KexMessage ret;
		ret.error_code = FAILED;
		return ret;
	}
	
	client = new rpc::client(kexReceived.ipaddress, kexReceived.port);
	
	return kex;
}



Future<uint32_t> AppState::SendMessage(std::string message) {
	SendFile(message);
	return SendEncryptedPacket<uint32_t>("Message", MSG, message.data(),
			message.size());
}

uint32_t AppState::ReceiveMessage(Message message) {
	std::vector<uint8_t> plaintext;
	bool res = DecryptMessage(message, plaintext);
	if(res) {
		if(message.msg_type == MSG) {
			std::string msg(plaintext.begin(), plaintext.end());
			PushMessage(msg);
			return SUCCESS;
		}
	}
	return INVALID_FUNCTION_PER_TYPE;
}

void AppState::PushMessage(const std::string& message) {
	std::lock_guard<std::mutex> l{mutex};
	receivedMessages.push(message);
}

bool AppState::PopMessage(std::string& message) {
	std::lock_guard<std::mutex> l{mutex};
	if(receivedMessages.empty())
		return false;
	message = receivedMessages.front();
	receivedMessages.pop();
	return true;
}


std::shared_ptr<Filestate> AppState::SendFile(std::string fileName) {
	std::shared_ptr<Filestate> fs = std::make_shared<Filestate>(fileName);
	filestate = fs;
	if(fs->Valid() == false) {
		return NULL;
	}
	fs->SendMeta();
	return fs;
}

uint32_t AppState::ReceiveFileMeta(Message message) {
	std::vector<uint8_t> plaintext;
	bool res = DecryptMessage(message, plaintext);
	if(res) {
		if(message.msg_type == FILE_META) {
			size_t size = *(uint64_t*)(plaintext.data());
			std::string name = (char*)(plaintext.data()+8);
			filestate = std::make_shared<Filestate>(name, size);
			if(filestate->Valid()) {
				return 0;
			} else {
				filestate = NULL;
				return 1;
			}
		}
	}
	return 2;
}

int32_t AppState::ReceiveFileBlock(Message message) {
	std::vector<uint8_t> plaintext;
	bool res = DecryptMessage(message, plaintext);
	if(res) {
		if(message.msg_type == FILE_BLOCK) {
			if(filestate == NULL) {
				return -1;
			}
			return filestate->UpdateReceive(plaintext.data(), plaintext.size());
		}
	}
	return -2;
}



void AppState::EncryptMessage(MSG_TYPE type, const void* plaintext,
		size_t length, Message& message) {
	message.cipher_variant = currentEncryptionMode;
	message.msg_type = type;
	uint8_t ad[2] = {(uint8_t)message.msg_type,
		(uint8_t)message.cipher_variant};
	Encrypt(plaintext, length, message.nonce, message.encrypted_data, ad, 2,
			message.cipher_variant);
}

bool AppState::DecryptMessage(const Message& message,
		std::vector<uint8_t>& plaintext) {
	uint8_t ad[2] = {(uint8_t)message.msg_type,
		(uint8_t)message.cipher_variant};
	return Decrypt(message.encrypted_data.data(), message.encrypted_data.size(),
		   message.nonce, plaintext, ad, 2, message.cipher_variant);	
}



void AppState::Encrypt(const void* plaintext, size_t plaintextLength,
		ChachaNonce& nonce, std::vector<uint8_t>& ciphertext,
		const void* ad, size_t adLength, ENCRYPTION_MODE encryptionMode) {
	Random::Fill(nonce.data(), nonce.size());
	if(encryptionMode == CHACHA20) {
		ciphertext.resize(plaintextLength);
		chacha::crypt(sharedKey.data(), nonce.data(), plaintext,
				ciphertext.data(), plaintextLength, 0);
	} else if(encryptionMode == CHACHA20_POLY1305) {
		ciphertext.resize(plaintextLength + chacha::MAC_SIZE);
		chacha::encrypt(sharedKey.data(), nonce.data(), plaintext,
				ciphertext.data(), plaintextLength, ad, adLength);
	}
}

bool AppState::Decrypt(const void* ciphertext, size_t ciphertextLength,
		const ChachaNonce& nonce, std::vector<uint8_t>& plaintext,
		const void* ad, size_t adLength, ENCRYPTION_MODE encryptionMode) {
	if(encryptionMode == CHACHA20) {
		plaintext.resize(ciphertextLength);
		chacha::crypt(sharedKey.data(), nonce.data(), ciphertext,
				plaintext.data(), ciphertextLength, 0);
		return true;
	} else if(encryptionMode == CHACHA20_POLY1305) {
		plaintext.resize(ciphertextLength - chacha::MAC_SIZE);
		auto r = chacha::decrypt(sharedKey.data(), nonce.data(), ciphertext,
				plaintext.data(), ciphertextLength, ad, adLength);
		if(r > 0)
			return true;
	}
	return false;
}

