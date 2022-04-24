#include "CryptoObjects.h"

#include <stdint.h>

#include <chacha-portable/chacha-portable.c>
void Chacha20Block(uint32_t* state) {
	core_block(state, state);
}
