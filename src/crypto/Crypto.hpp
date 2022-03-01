
#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <errno.h>

#include "Random.hpp"

namespace ec {
	bool GenKey(uint8_t* privkey32, uint8_t* pubkey33);
	bool Sign(uint8_t* privkey32, uint8_t* hash32, uint8_t* sign64);
	bool Verify(uint8_t* pubkey33, uint8_t* hash32, uint8_t* sign64);
	bool Ecdh(uint8_t* myPrivKey32, uint8_t* theirPubKey33, uint8_t* shared32);
	bool Ecdhe(uint8_t* theirPubKey33, uint8_t* myPubKey33, uint8_t* shared32);
}

namespace chacha {
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
	
}

#endif

