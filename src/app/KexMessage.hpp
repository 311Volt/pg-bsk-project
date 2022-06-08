#ifndef SRC_APP_KEXMESSAGE
#define SRC_APP_KEXMESSAGE

#include "Codes.hpp"
#include "AppState.hpp"

struct KexMessage {
	ERROR_CODE error_code;
	EcPublicKey publicKey;
	EcPublicKey publicEcdheKey;
	std::string ipaddress;
	int port;
	EcSignature signature;
	
	void GenerateDigest(Array32& hash);
	bool Verify();
	bool Sign(EcPrivateKey& privkey);

	MSGPACK_DEFINE_ARRAY(error_code, publicKey, publicEcdheKey, ipaddress, port, signature);
};


#endif /* SRC_APP_KEXMESSAGE */
