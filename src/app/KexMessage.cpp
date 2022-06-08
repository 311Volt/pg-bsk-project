#include "KexMessage.hpp"

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
