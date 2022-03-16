
#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <errno.h>

#include "Random.hpp"
#include <digestpp.hpp>

namespace ec {
	inline const static int PRIVATE_KEY_SIZE = 32;
	inline const static int PUBLIC_KEY_SIZE = 33;
	inline const static int SHARED_SECRET_SIZE = 32;
	inline const static int HASH_TO_SIGN_SIZE = 32;
	inline const static int SIGNATURE_SIZE = 64;
	
	bool GenKey(uint8_t* privkey32, uint8_t* pubkey33);
	bool Sign(const uint8_t* privkey32, const uint8_t* hash32, uint8_t* sign64);
	bool Verify(const uint8_t* pubkey33, const uint8_t* hash32, const uint8_t* sign64);
	bool Sign(const uint8_t* privkey32, const uint8_t* msg, size_t msglen, uint8_t* sign64);
	bool Verify(const uint8_t* pubkey33, const uint8_t* msg, size_t msglen, const uint8_t* sign64);
	bool Ecdh(const uint8_t* myPrivKey32, const uint8_t* theirPubKey33, uint8_t* shared32);
	bool Ecdhe(const uint8_t* theirPubKey33, uint8_t* myPubKey33, uint8_t* shared32);
}

namespace chacha {
	inline const static int KEY_SIZE = 32;
	inline const static int NONCE_SIZE = 12;
	inline const static int MAC_SIZE = 16;
	
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
	inline const static int KEY_SIZE = 32;
	inline const static int MAC_SIZE = 16;
	
	void poly(const uint8_t key32, const uint8_t* buffer, size_t bytes,
			uint8_t* mac16);
}

class digest {
public:
	template<uint32_t bits>
	class sha {
	public:
		inline const static uint32_t bytes = bits/8;
		inline const static int BYTES = bytes;
		inline const static int BITS = bits;
		
		inline sha(const uint8_t* data, size_t size, uint8_t digest[bytes]) :
			hash(bits) {
			absorb(data, size).finalize(digest);
		}
		inline sha(const uint8_t* data, size_t size) :
			hash(bits) {
			absorb(data, size);
		}
		inline sha() : hash(bits) {}
		inline sha& absorb(const uint8_t* data, size_t size) {
			hash.absorb(data, size);
			return *this;
		}
		inline void finalize(uint8_t digest[bytes]) const {
			hash.digest(digest, bytes);
		}
		
		digestpp::sha3 hash;
	};
	
	using sha256 = sha<256>;
	using sha384 = sha<384>;
	using sha512 = sha<512>;
};

#endif

