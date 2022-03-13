
#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <errno.h>

#include "Random.hpp"

namespace ec {
	bool GenKey(uint8_t* privkey32, uint8_t* pubkey33);
	bool Sign(const uint8_t* privkey32, const uint8_t* hash32, uint8_t* sign64);
	bool Verify(const uint8_t* pubkey33, const uint8_t* hash32, const uint8_t* sign64);
	bool Sign(const uint8_t* privkey32, const uint8_t* msg, size_t msglen, uint8_t* sign64);
	bool Verify(const uint8_t* pubkey33, const uint8_t* msg, size_t msglen, const uint8_t* sign64);
	bool Ecdh(const uint8_t* myPrivKey32, const uint8_t* theirPubKey33, uint8_t* shared32);
	bool Ecdhe(const uint8_t* theirPubKey33, uint8_t* myPubKey33, uint8_t* shared32);
}

namespace chacha {
	void crypt(const uint8_t* key32, const uint8_t* nonce12,
			const uint8_t* src, uint8_t* dst, uint32_t length,
			uint32_t counter);
	void encrypt(const uint8_t* key32, const uint8_t* nonce12,
			const uint8_t* plaintext, uint8_t* ciphertextWithTag,
			uint32_t plaintextLength, const uint8_t* ad, size_t adSize);
	uint32_t decrypt(const uint8_t* key32, const uint8_t* nonce12,
			const uint8_t* ciphertextWithTag, uint8_t* decryptedPlaintext,
			uint32_t ciphertextWithTagSize, uint8_t* ad, size_t adSize);
}

namespace poly {
	void poly(const uint8_t key32, const uint8_t* buffer, size_t bytes,
			uint8_t* mac16);
}

namespace digest {
	void sha256(const uint8_t* data, size_t size, uint8_t* digest32);
	void sha384(const uint8_t* data, size_t size, uint8_t* digest48);
	void sha512(const uint8_t* data, size_t size, uint8_t* digest64);
}

#endif

