
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
// #define HAVE_STDIO_H 1
// #define HAVE_STDLIB_H 1
// #define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define ECMULT_GEN_PREC_BITS 4
#define ECMULT_WINDOW_SIZE 15
#define ENABLE_MODULE_ECDH
#define ENABLE_MODULE_RECOVERY
#define SECP256K1_BUILD


/* Define this symbol to enable x86_64 assembly optimizations */
#define USE_ASM_X86_64 1

/* Define this symbol if an external (non-inline) assembly implementation is
   used */
/* #undef USE_EXTERNAL_ASM */

/* Define this symbol if an external implementation of the default callbacks
   is used */
/* #undef USE_EXTERNAL_DEFAULT_CALLBACKS */

/* Define this symbol to force the use of the (unsigned) __int128 based wide
   multiplication implementation */
// #define USE_FORCE_WIDEMUL_INT128 1

/* Define this symbol to force the use of the (u)int64_t based wide
   multiplication implementation */
// #define USE_FORCE_WIDEMUL_INT64 1


#include "../../secp256k1/src/precomputed_ecmult.c"
#include "../../secp256k1/src/secp256k1.c"
#include "../../secp256k1/src/precomputed_ecmult_gen.c"

#include "../../secp256k1/src/modules/extrakeys/main_impl.h"
#include "../../secp256k1/src/modules/schnorrsig/main_impl.h"

#include "../../portable8439/src/portable8439.c"
#include "../../portable8439/src/chacha-portable/chacha-portable.c"
#include "../../portable8439/src/poly1305-donna/poly1305-donna.c"

void Chacha20Block(uint32_t* state) {
	core_block(state, state);
}

