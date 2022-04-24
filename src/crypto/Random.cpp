
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>

#include "Random.hpp"
#include "CryptoObjects.h"

Random randomInstance;
thread_local const uint32_t thread_id = ++randomInstance.threadCounter;
thread_local uint64_t threadLocalCounter;
thread_local std::random_device stdRandom;


Random::State::State() noexcept {
}

void Random::State::XorTimeRandom() {
	union {
		time_t t = time(NULL);
		uint32_t some[(sizeof(time_t)+3)/4];
	};
	for(int i=0; i<sizeof(some)/4; ++i)
		m[i] ^= some[i];
	m[15] ^= thread_id;
	*((uint64_t*)&(m[5])) ^= rand();
	*((uint64_t*)&(m[6])) ^= stdRandom();
	*((uint64_t*)&(m[8])) ^= ++randomInstance.nonUatomicBetweenThreadsUsageCounter;
	*((decltype(clock())*)&(m[10])) ^= clock();
	*((uint64_t*)&(m[12])) ^= ++threadLocalCounter;
	*((uint64_t*)&(m[14])) ^= ++randomInstance.atomicUsageCounter;
}

#ifdef __unix__
#include <sys/random.h>
#endif

void Random::State::XorWithdevrandom() {
#ifdef __unix__
	union {
		uint8_t r[64];
		uint32_t r32[16];
	};
	
	getrandom(r, 64, GRND_RANDOM);
	for(int i=0; i<16; ++i)
		m[i] ^= r32[i];
	
	FILE* devrandom = fopen("/dev/random", "r");
	if(devrandom) {
		fread(r, 1, 64, devrandom);
		for(int i=0; i<16; ++i)
			m[i] ^= r32[i];
	}
	
	memset(r, 0, 64);
#endif
}

Random::State::~State() {
	memset(m, 0, sizeof(State));
}

Random::State& Random::State::operator^=(const Random::State& other) {
	for(int i=0; i<16; ++i) {
		uint32_t v = __atomic_xor_fetch((uint32_t*)other.m+i, 0, 0);
		__atomic_xor_fetch(m+i, v, 0);
	}
	return *this;
}


Random::Random() {
}

Random::~Random() {
}

void Random::Fill(void* ptr, size_t size) {
	State state;
	state ^= randomInstance.state;
	state.XorTimeRandom();
	state.XorWithdevrandom();
	Chacha20Block(state.m);
	
	uint32_t* ptr32 = (uint32_t*)ptr;
	size_t fullBlocks = size/(16*4);
	for(size_t i=0; i<fullBlocks; ++i, ptr32+=16, size-=16*4) {
		for(int j=0; j<16; ++j) {
			ptr32[i] ^= state.m[i];
		}
		state.XorTimeRandom();
		Chacha20Block(state.m);
	}
	uint8_t* ptr8 = (uint8_t*)ptr32;
	size_t rest = size - fullBlocks*16*4;
	if(rest > 0) {
		uint8_t* state_ptr = (uint8_t*)state.m;
		for(size_t i=0; i<rest; ++i) {
			ptr8[i] ^= state_ptr[i];
		}
		state.XorTimeRandom();
		Chacha20Block(state.m);
	}

	randomInstance.state ^= state;
}

