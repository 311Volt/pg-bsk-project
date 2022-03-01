
#include "../src/crypto/Crypto.hpp"

#include <cstdio>

int main() {
	uint8_t privA[32], privB[32], pubA[33], pubB[33], sign[64];
	uint8_t sharedA[32], sharedB[32];
	ec::GenKey(privA, pubA);
	ec::GenKey(privB, pubB);
	
	ec::Ecdh(privA, pubB, sharedA);
	ec::Ecdh(privB, pubA, sharedB);
	
	ec::Sign(privA, sharedA, sign);
	bool res = ec::Verify(pubA, sharedB, sign);
	
	printf(" ecdh and ecdsa ... %s\n", res ? "OK" : "FAILED"); 
	
	uint8_t plaintext[128];
	uint8_t ciphertext[128+16];
	uint8_t decoded[128];
	
	uint8_t nonce[12], aad[32];
	Random::Fill(nonce, sizeof(nonce));
	Random::Fill(aad, sizeof(aad));
	
	chacha::encrypt(sharedA, nonce, plaintext, ciphertext, sizeof(plaintext),
			aad, sizeof(aad));
	
	res = chacha::decrypt(sharedB, nonce, ciphertext, decoded,
			sizeof(ciphertext), aad, sizeof(aad));
	
	printf(" decrypting ... %s\n", res ? "OK" : "FAILED"); 
	
	res = !memcmp(plaintext, decoded, sizeof(plaintext));
	
	printf(" decrypted data is ... %s\n", res ? "OK" : "FAILED"); 
	
	return 0;
}

