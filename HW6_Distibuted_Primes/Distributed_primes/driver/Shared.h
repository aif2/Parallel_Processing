/*#pragma once
#include <array>

const int PRIMES_COUNT = 1000000;
const int PRIMES_RANGE = 1000;*/


#pragma once

#include <array>

#define PRIMES_QUEUE_SIZE 100
#define PRIMES_PER_PROCESS 100000

namespace cs477 {
	using shared_prime_request = unsigned int;

	template<unsigned int N>
	using shared_prime_reply = std::array<unsigned int, N>;
}