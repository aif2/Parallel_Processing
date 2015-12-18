#include "stdafx.h"

#include <algorithm>
#include <cmath>
#include "../../../include/cs477.h"
#include "../driver/Shared.h"

using namespace cs477;

std::shared_ptr<bounded_queue<shared_prime_request, PRIMES_QUEUE_SIZE>> rq_queue;
std::shared_ptr<bounded_queue<shared_prime_reply<PRIMES_PER_PROCESS>, PRIMES_QUEUE_SIZE>> rp_queue;

bool is_prime(int n) {
	auto j = static_cast<int>(sqrt(n));
	for (int i = 2; i <= j; i++) {
		if (n % i == 0) return false;
	}
	return true;
}

int main() {

	// Create shared resources.

	rq_queue = std::make_shared<bounded_queue<shared_prime_request, PRIMES_QUEUE_SIZE>>();
	rq_queue->create("primes-rq-queue");

	rp_queue = std::make_shared<bounded_queue<shared_prime_reply<PRIMES_PER_PROCESS>, PRIMES_QUEUE_SIZE>>();
	rp_queue->create("primes-rp-queue");

	// Setup constants
	using future_of_primes = future<std::vector<int>>;

	const unsigned int thread_count = std::thread::hardware_concurrency() * 5;
	const unsigned int elements = PRIMES_PER_PROCESS;
	const unsigned int thread_unit = elements / thread_count;

	for (;;) {
		// Read a request
		auto request_index = rq_queue->read();

		std::vector<future_of_primes> futures;

		try {
			for (unsigned int i = request_index; i < thread_count + request_index; ++i) {
				auto future = queue_work([i, &thread_unit] {
					std::vector<int> vec;

					for (unsigned int k = i * thread_unit; k < (i + 1) * thread_unit; ++k) {
						if (is_prime(k)) {
							vec.push_back(k);
						}
					}
					return vec;
				});
				futures.push_back(std::move(future));
			}

			future<std::vector<future_of_primes>> result = when_all(futures.begin(), futures.end());

			shared_prime_reply<PRIMES_PER_PROCESS> response_val;

			auto it = response_val.begin();
			for (auto&& future : result.get()) {
				auto res = future.get();
				std::copy(res.begin(), res.end(), it);
				std::advance(it, res.size());
			}
			*it = 0;

			rp_queue->write(response_val);
		}
		catch (std::system_error &ex) {
			printf("Error: %d (%s)\n", ex.code().value(), ex.what());
		}
		catch (std::exception &ex) {
			printf("Error: %s\n", ex.what());
		}
		catch (...) {
			printf("Error!\n");
		}
	}

	return 0;
}