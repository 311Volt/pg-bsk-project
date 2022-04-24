

file(GLOB_RECURSE secp256k1_sources "secp256k1/src/secp256k1.c" "secp256k1/src/precomputed_ecmult.c" "secp256k1/src/precomputed_ecmult_gen.c")
add_library(secp256k1_bsk STATIC ${secp256k1_sources})
target_compile_definitions(secp256k1_bsk PUBLIC ECMULT_WINDOW_SIZE=15)
target_compile_definitions(secp256k1_bsk PUBLIC ECMULT_GEN_PREC_BITS=4)
target_compile_definitions(secp256k1_bsk PUBLIC ENABLE_MODULE_ECDH)
target_compile_definitions(secp256k1_bsk PUBLIC ENABLE_MODULE_RECOVERY)
target_compile_definitions(secp256k1_bsk PUBLIC ENABLE_MODULE_EXTRAKEYS)
target_compile_definitions(secp256k1_bsk PUBLIC ENABLE_MODULE_SCHNORRSIG)
target_compile_definitions(secp256k1_bsk PUBLIC USE_ASM_X86_64=1)
