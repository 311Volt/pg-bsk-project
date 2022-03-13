
#include "Crypto.hpp"

#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define ECMULT_GEN_PREC_BITS 4
#define ECMULT_WINDOW_SIZE 15
#define ENABLE_MODULE_ECDH
#define ENABLE_MODULE_RECOVERY
#define SECP256K1_BUILD

#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <cstdlib>
#include <ctime>

#include "../../secp256k1/include/secp256k1.h"
#include "../../secp256k1/include/secp256k1_ecdh.h"
#include "../../secp256k1/include/secp256k1_schnorrsig.h"
#include "../../secp256k1/include/secp256k1_extrakeys.h"

class Ctx {
public:
	inline Ctx() {
		ctx = secp256k1_context_create(
				SECP256K1_CONTEXT_SIGN |
				SECP256K1_CONTEXT_VERIFY |
				SECP256K1_CONTEXT_DECLASSIFY);
	}
	inline ~Ctx() {
		secp256k1_context_destroy(ctx);
	}
	secp256k1_context* ctx = NULL;
};

Ctx ctx;

namespace ec {
	bool GenKey(uint8_t* privkey32, uint8_t* pubkey33) {
		do {
			Random::Fill(privkey32, 32);
		} while(!secp256k1_ec_seckey_verify(ctx.ctx, privkey32));
		secp256k1_pubkey pubkey_;
		int res1 = (1-secp256k1_ec_pubkey_create(ctx.ctx, &pubkey_, privkey32)) << 1;
		size_t len=33;
		int res2 = 1-secp256k1_ec_pubkey_serialize(ctx.ctx, pubkey33, &len, &pubkey_,
				SECP256K1_EC_COMPRESSED);
		if(res1 | res2) {
			errno = res1 | res2;
			return false;
		}
		return true;
	}

	bool Sign(const uint8_t* privkey32, const uint8_t* hash32,
			uint8_t* sign64) {
		uint8_t r[32];
		Random::Fill(r, 32);
		secp256k1_keypair keypair;
		int res = 1-secp256k1_keypair_create(ctx.ctx, &keypair, privkey32);
		secp256k1_schnorrsig_sign(ctx.ctx, sign64, hash32, &keypair, r);
		memset(&keypair, 0, sizeof(keypair));
		if(res) {
			errno = res;
			return false;
		}
		return true;
	}

	bool Verify(const uint8_t* pubkey33, const uint8_t* hash32,
			const uint8_t* sign64) {
		secp256k1_xonly_pubkey xpubkey;
		//int res0 = 4;

		// TODO: check: can I just ignore first byte of pubkey33?
		// 	secp256k1_pubkey pubkey_;
		// 	res0 = (1-secp256k1_ec_pubkey_parse(ctx.ctx, &pubkey_, pubkey33, 33))<<2;
		// 	int res1 = (1-secp256k1_xonly_pubkey_from_pubkey(ctx.ctx, &xpubkey, NULL, &pubkey_))<<1;
		int res1 = (1-secp256k1_xonly_pubkey_parse(ctx.ctx, &xpubkey, pubkey33+1))<<1;

		int res2 = 1-secp256k1_schnorrsig_verify(ctx.ctx, sign64, hash32, 32, &xpubkey);
		if(res1 | res2) {
			errno = res1 | res2;
			return false;
		}
		return true;
	}
	
	bool Sign(const uint8_t* privkey32, const uint8_t* msg, size_t msglen,
			uint8_t* sign64) {
		uint8_t hash[32];
		digest::sha256(msg, msglen, hash);
		return Sign(privkey32, hash, sign64);
	}
	
	bool Verify(const uint8_t* pubkey33, const uint8_t* msg, size_t msglen,
			const uint8_t* sign64) {
		uint8_t hash[32];
		digest::sha256(msg, msglen, hash);
		return Verify(pubkey33, hash, sign64);
	}

	bool Ecdh(const uint8_t* myPrivKey32, const uint8_t* theirPubKey33,
			uint8_t* shared32) {
		secp256k1_pubkey pubkey;
		int res1 = (1-secp256k1_ec_pubkey_parse(ctx.ctx, &pubkey, theirPubKey33, 33))<<1;
		int res2 = 1-secp256k1_ecdh(ctx.ctx, shared32, &pubkey, myPrivKey32, NULL, NULL);
		if(res1 | res2) {
			errno = res1 | res2;
			return false;
		}
		return true;
	}

	bool Ecdhe(const uint8_t* theirPubKey33, uint8_t* myPubKey33,
			uint8_t* shared32) {
		uint8_t privkey32[32];
		bool ret = GenKey(privkey32, myPubKey33);
		if(ret == false)
			return false;
		ret = Ecdh(privkey32, theirPubKey33, shared32);
		memset(privkey32, 0, 32);
		return ret;
	}
}

extern "C" {
#include "../../portable8439/src/poly1305-donna/poly1305-donna.h"
#include "../../portable8439/src/portable8439.h"
#include "../../portable8439/src/chacha-portable/chacha-portable.h"
}

namespace chacha {
	void crypt(const uint8_t* key32, const uint8_t* nonce12,
			const uint8_t* src, uint8_t* dst, uint32_t length,
			uint32_t counter) {
		chacha20_xor_stream(dst, src, length, key32, nonce12, counter);
	}
	
	void encrypt(const uint8_t* key32, const uint8_t* nonce12,
			const uint8_t* plaintext, uint8_t* ciphertextWithTag,
			uint32_t plaintextLength, const uint8_t* ad, size_t adSize) {
		portable_chacha20_poly1305_encrypt(ciphertextWithTag, key32, nonce12,
				ad, adSize, plaintext, plaintextLength);
	}
	
	uint32_t decrypt(const uint8_t* key32, const uint8_t* nonce12,
			const uint8_t* ciphertextWithTag, uint8_t* decryptedPlaintext,
			uint32_t ciphertextWithTagSize, uint8_t* ad, size_t adSize) {
		size_t num = portable_chacha20_poly1305_decrypt(decryptedPlaintext, key32,
				nonce12, ad, adSize, ciphertextWithTag, ciphertextWithTagSize);
		if(num == -1)
			return 0;
		return num;
	}
}

namespace poly {
	void poly(const uint8_t* key32, const uint8_t* buffer, size_t bytes,
			uint8_t* mac16) {
		poly1305_context ctx;
		poly1305_init(&ctx, key32);
		poly1305_update(&ctx, buffer, bytes);
		poly1305_finish(&ctx, mac16);
	}
}

#include "../../digestpp/digestpp.hpp"

namespace digest {
	void sha256(const uint8_t* data, size_t size, uint8_t* digest32) {
		digestpp::sha3(256).absorb(data, size).digest(digest32, 32);
	}
	
	void sha384(const uint8_t* data, size_t size, uint8_t* digest48) {
		digestpp::sha3(384).absorb(data, size).digest(digest48, 48);
	}
	
	void sha512(const uint8_t* data, size_t size, uint8_t* digest64) {
		digestpp::sha3(512).absorb(data, size).digest(digest64, 64);
	}
}

