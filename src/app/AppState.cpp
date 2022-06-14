
#include <fstream>
#include <cmath>
#include <functional>
#include <memory>

#include "AppState.hpp"
#include "KexMessage.hpp"
#include "crypto/Crypto.hpp"

#include <fmt/format.h>

AppState* AppState::singleton = nullptr;
std::atomic<size_t> __PID = 0;
thread_local size_t PID = []()->size_t{if(__PID>30)exit(-311);return ++__PID;}();

AppState::AppState(std::string myIp, int32_t port) : 
	rpcServer(port),
	ipAddress(myIp), 
	port(port) 
{
	client = nullptr;
	GenerateKey();
	BindAll();
	rpcServer.async_run(1);
	currentEncryptionMode = ENCRYPTION_MODE::CHACHA20_POLY1305;
	singleton = this;
}

AppState::~AppState() 
{
	if(singleton == this)
		singleton = nullptr;
}

void AppState::BindAll() 
{
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

bool AppState::GenerateKey() {
	return ec::GenKey(this->privateKey.data(), this->publicKey.data());
}

bool AppState::LoadPrivateKey(std::string keyFilePath,
		const std::string& passphrase) {
	std::ifstream file(keyFilePath);
	if(file.good()) {
		Array32 fkey, saltA, saltB, saltC;
		file.read((char*)saltA.data(), saltA.size());
		file.read((char*)saltB.data(), saltB.size());
		file.read((char*)saltC.data(), saltC.size());
		
		digest::sha256()
			.absorb(saltA)
			.absorb(passphrase.data(), passphrase.size())
			.absorb(saltB)
			.finalize(fkey.data());
		digest::sha256()
			.absorb(fkey)
			.absorb(saltC)
			.finalize(fkey.data());
		
		ChachaNonce nonce;
		file.read((char*)nonce.data(), nonce.size());
		
		std::array<uint8_t, chacha::MAC_SIZE+ec::PRIVATE_KEY_SIZE> cipheredKey;
		EcPrivateKey key;
		file.read((char*)cipheredKey.data(), cipheredKey.size());
		if(chacha::decrypt(fkey.data(), nonce.data(), cipheredKey.data(),
					key.data(), cipheredKey.size(), NULL, 0) == false)
			return false;
		this->privateKey = key;
		return ec::DerivePublicKey(this->privateKey.data(),
				this->publicKey.data());
	}
	return false;
}

bool AppState::SavePrivateKey(std::string keyFilePath,
		const std::string& passphrase) const {
	std::ofstream file(keyFilePath);
	if(file.good()) {
		Array32 fkey, saltA, saltB, saltC;
		Random::Fill(saltA.data(), saltA.size());
		Random::Fill(saltB.data(), saltB.size());
		Random::Fill(saltC.data(), saltC.size());
		file.write((char*)saltA.data(), saltA.size());
		file.write((char*)saltB.data(), saltB.size());
		file.write((char*)saltC.data(), saltC.size());
		
		digest::sha256()
			.absorb(saltA)
			.absorb(passphrase.data(), passphrase.size())
			.absorb(saltB)
			.finalize(fkey.data());
		digest::sha256()
			.absorb(fkey)
			.absorb(saltC)
			.finalize(fkey.data());
		
		ChachaNonce nonce;
		Random::Fill(nonce.data(), nonce.size());
		file.write((char*)nonce.data(), nonce.size());
		
		std::array<uint8_t, chacha::MAC_SIZE+ec::PRIVATE_KEY_SIZE> cipheredKey;
		chacha::encrypt(fkey.data(), nonce.data(), this->privateKey.data(),
				cipheredKey.data(), this->privateKey.size(), NULL, 0);
		file.write((char*)cipheredKey.data(), cipheredKey.size());
		return true;
	}
	return false;
}

bool AppState::LoadPublicKey(std::string keyFilePath) {
	std::ifstream file(keyFilePath);
	if(file.good()) {
		file.read((char*)publicKey.data(), publicKey.size());
		return true;
	}
	return false;
}

bool AppState::SavePublicKey(std::string keyFilePath) const {
	std::ofstream file(keyFilePath);
	if(file.good()) {
		file.write((const char*)publicKey.data(), publicKey.size());
		return true;
	}
	return false;
}

std::string AppState::GetKeyFingerprint() const
{
	Array32 privKeyHash;
	digest::sha256().absorb(privateKey).finalize(privKeyHash.data());

	std::string fingerprint;
	for(uint8_t b: privKeyHash) {
		fingerprint += fmt::format("{:02x}", b);
	}

	return fingerprint;
}

ERROR_CODE AppState::ConnectAndHandshake(std::string ip, int32_t port) 
{
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
			if(ec::Ecdh(
				privateEcdheKey.data(),
				kexResponse.publicEcdheKey.data(), 
				sharedKey.data()
			)) {
				theirPublicKey = kexResponse.publicEcdheKey;
			} else {
				throw KexError("Key exchange failed");
			}
		} else {
			throw KexError("Response verification failed");
		}
	} else {
		throw KexError("Key exchange was rejected by peer");
	}

	theirIPAddress = kexResponse.ipaddress;
	theirPort = kexResponse.port;

	client = std::move(newClient);
	return SUCCESS;
}

KexMessage AppState::ReceiveKex(KexMessage kexReceived) 
{
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
	
	client = std::make_unique<rpc::client>(kexReceived.ipaddress, kexReceived.port);

	theirIPAddress = kexReceived.ipaddress;
	theirPort = kexReceived.port;

	if(onReceiveKex)
		onReceiveKex();
	return kex;
}

void AppState::SetReceiveKexCallback(std::function<void(void)> fn)
{
	onReceiveKex = fn;
}

Future<uint32_t> AppState::SendMessage(std::string message)
{
	return SendEncryptedPacket<uint32_t>("Message", MSG, message.data(), message.size());
}

uint32_t AppState::ReceiveMessage(Message message)
{
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

void AppState::PushMessage(const std::string& message)
{
	std::lock_guard<std::mutex> l{mutex};
	receivedMessages.push(message);
}

bool AppState::PopMessage(std::string& message)
{
	std::lock_guard<std::mutex> l{mutex};
	if(receivedMessages.empty())
		return false;
	message = receivedMessages.front();
	receivedMessages.pop();
	return true;
}


std::shared_ptr<Filestate> AppState::SendFile(std::string fileName)
{
	std::shared_ptr<Filestate> fs = std::make_shared<Filestate>(fileName);
	fs->self = fs;
	if(fs->Valid() == false) {
		return nullptr;
	}
	fs->SendMeta();
	return fs;
}

uint32_t AppState::ReceiveFileMeta(Message message)
{
	std::vector<uint8_t> plaintext;
	bool res = DecryptMessage(message, plaintext);
	if(res) {
		if(message.msg_type == FILE_META) {
			size_t size = *(uint64_t*)(plaintext.data());
			void* hash = plaintext.data()+8;
			std::string name = (char*)(plaintext.data()+8+32);
			filestate = std::make_shared<Filestate>(hash, name, size);
			if(filestate->Valid()) {
				return 0;
			} else {
				filestate = nullptr;
				return 1;
			}
		}
	}
	return 2;
}

int32_t AppState::ReceiveFileBlock(Message message)
{
	std::vector<uint8_t> plaintext;
	bool res = DecryptMessage(message, plaintext);
	if(res) {
		if(message.msg_type == FILE_BLOCK) {
			if(filestate == nullptr) {
				return -1;
			}
			return filestate->UpdateReceive(plaintext.data(), plaintext.size());
		}
	}
	return -2;
}



void AppState::EncryptMessage(MSG_TYPE type, const void* plaintext,
		size_t length, Message& message)
{
	message.cipher_variant = currentEncryptionMode;
	message.msg_type = type;
	uint8_t ad[2] = {(uint8_t)message.msg_type,
		(uint8_t)message.cipher_variant};
	Encrypt(plaintext, length, message.nonce, message.encrypted_data, ad, 2,
			message.cipher_variant);
}

bool AppState::DecryptMessage(const Message& message,
		std::vector<uint8_t>& plaintext) 
{
	uint8_t ad[2] = {(uint8_t)message.msg_type,
		(uint8_t)message.cipher_variant};
	return Decrypt(message.encrypted_data.data(), message.encrypted_data.size(),
		   message.nonce, plaintext, ad, 2, message.cipher_variant);	
}



void AppState::Encrypt(const void* plaintext, size_t plaintextLength,
		ChachaNonce& nonce, std::vector<uint8_t>& ciphertext,
		const void* ad, size_t adLength, ENCRYPTION_MODE encryptionMode) 
{
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
		const void* ad, size_t adLength, ENCRYPTION_MODE encryptionMode) 
{
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

