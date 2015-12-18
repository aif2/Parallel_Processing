// driver.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "Shared.h"
#include "../../../include/cs477.h"
#include <iostream>
using namespace cs477;

std::shared_ptr<bounded_queue<shared_prime_request, PRIMES_QUEUE_SIZE>> rq_queue;
std::shared_ptr<bounded_queue<shared_prime_reply<PRIMES_PER_PROCESS>, PRIMES_QUEUE_SIZE>> rp_queue;

int main() {

	// Create shared resources.

	rq_queue = std::make_shared<bounded_queue<shared_prime_request, PRIMES_QUEUE_SIZE>>();
	rq_queue->create("primes-rq-queue");

	rp_queue = std::make_shared<bounded_queue<shared_prime_reply<PRIMES_PER_PROCESS>, PRIMES_QUEUE_SIZE>>();
	rp_queue->create("primes-rp-queue");

	for (int i = 0; i < PRIMES_QUEUE_SIZE; ++i) {
		rq_queue->write(i);
	}

	for (;;) {
		// Read a response
		auto response = rp_queue->read();

		bool first = true;
		for (auto&& entry : response) {
			if (entry == 0) {
				break;
			}

			if (first) {
				first = false;
				std::cout << entry;
				continue;
			}
			std::cout << ", " << entry;
		}
	}


	return 0;
}