
#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cinttypes>
#include <atomic>
#include <cstring>
#include <ctime>

class Random {
public:
	struct State {
		uint32_t m[16];
		State() noexcept;
		State(State&) = delete;
		State(const State&) = delete;
		State(State&&) = delete;
		State& operator =(State&) = delete;
		State& operator =(const State&) = delete;
		State& operator =(State&&) = delete;
		State& operator^=(const State& other);
		~State();
		void XorTimeRandom();
		void XorWithdevrandom();
	};
	
	Random();
	~Random();
	
	State state;
	std::atomic<uint32_t> threadCounter;
	std::atomic<uint64_t> atomicUsageCounter;
	uint64_t nonUatomicBetweenThreadsUsageCounter;
	
	static void Fill(void* ptr, size_t size);
};

#endif

