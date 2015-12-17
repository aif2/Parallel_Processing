
#include "stdafx.h"
#include "../../include/cs477.h"

using namespace cs477;

bool is_prime(int n)
{
	auto j = static_cast<int>(sqrt(n));
	for (int i = 2; i <= j; i++)
	{
		if (n % i == 0) return false;
	}
	return true;
}

int main()
{
	int NUM_THREADS = 4;
	int primesPerThread = 250000;
	std::vector<cs477::thread> threads;
	std::vector<std::vector<int>> result;

	try {



		for (int i = 0; i < NUM_THREADS; i++)
		{
			threads.push_back(cs477::create_thread([i, primesPerThread, &result]()
			{

				std::vector<int> primes;
				for (int j = i; j < 1000000; j += 4)
				{
					if (is_prime(j))
					{
						primes.push_back(j);
						printf("%d\n", j);
					}
				}
				result.push_back(primes);
			}));
		}

		for (auto &&thread : threads)
			join(thread);

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
		return 0;
	}

