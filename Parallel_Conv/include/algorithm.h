

#pragma once

#include "cs477.h"

#include <algorithm>
#include <thread>

namespace cs477
{

	struct parallel_threshold
	{
		static const size_t count = 1;
		static const size_t sum = 1;
	};

	template<class InputIt, class T>
	size_t parallel_count(InputIt first, InputIt last, const T& value)
	{

		if (last - first < parallel_threshold::count)
		{
			// Let the STL handle the small cases
			return std::count(first, last, value);
		}

		// Parition the work into n chunks.  
		// Each thread will work only on one chunk.
		auto threads = std::thread::hardware_concurrency();
		auto count = last - first;
		auto count_per_thread = count / threads;

		std::vector<future<size_t>> futures;

		auto chunk_first = first;
		for (unsigned i = 0; i < threads; i++)
		{
			auto chunk_last = chunk_first + count_per_thread;

			futures.push_back(queue_work([chunk_first, chunk_last, value]
			{
				// Compute the count of this chunk.
				size_t ret = 0;
				auto first = chunk_first;
				for (; first != chunk_last; ++first)
				{
					if (*first == value)
					{
						ret++;
					}
				}

				return ret;
			}));

			// Don't forget to move to the next chunk
			chunk_first = chunk_last;
		}

		// Wait for all the futures to finish.
		auto f = when_all(futures.begin(), futures.end());
		auto counts = f.get();
		
		// Compute the sum of the local values.
		size_t ret = 0;
		for (auto &&i : counts)
		{
			ret += i.get();
		}

		return ret;
	}




	template<class InputIt>
	auto sum(InputIt first, InputIt last)
	{
		using T = std::decay<decltype(*first)>::type;
		if (first != last)
		{
			T s = *first++;
			while (first != last)
			{
				s += *first++;
			}
			return s;
		}
		else
		{
			return T{};
		}
	}

	template<class InputIt>
	auto parallel_sum(InputIt first, InputIt last, ptrdiff_t threshold = parallel_threshold::sum)
	{
		//if (std::distance(first, last) < threshold)
		//{
		//	// Let the STL handle the small cases
		//	return sum(first, last);
		//}

		using T = std::decay<decltype(*first)>::type;

		// Parition the work into n chunks.  
		// Each thread will work only on one chunk.
		auto threads = std::thread::hardware_concurrency();
		auto count = last - first;
		auto count_per_thread = count / threads;

		std::vector<future<T>> futures;

		auto chunk_first = first;
		for (unsigned i = 0; i < threads; i++)
		{
			auto chunk_last = chunk_first + count_per_thread;

			futures.push_back(queue_work([chunk_first, chunk_last]
			{
				return sum(chunk_first, chunk_last);
			}));

			// Don't forget to move to the next chunk
			chunk_first = chunk_last;
		}

		// Wait for all the futures to finish.
		auto f = when_all(futures.begin(), futures.end());
		auto sums = f.get();

		// Compute the sum of the local values.
		T s{};
		for (auto &&i : sums)
		{
			s += i.get();
		}

		return s;
	}


	template<class Executor, class InputIt>
	auto parallel_sum(const Executor &ex, InputIt first, InputIt last)
	{
		using T = std::decay<decltype(*first)>::type;

		// Parition the work into n chunks.  
		// Each thread will work only on one chunk.
		auto threads = ex.concurrency();
		if (threads <= 1)
		{
			return sum(first, last);
		}

		std::vector<T> sums;
		sums.resize(threads);

		auto tb = ex.make_task_block();

		auto count = last - first;
		auto count_per_thread = count / threads;

		auto chunk_first = first;
		for (unsigned i = 0; i < threads; i++)
		{
			auto chunk_last = chunk_first + count_per_thread;

			auto result = &sums[i];
			tb.run([result, chunk_first, chunk_last]
			{
				*result = sum(chunk_first, chunk_last);
			});

			// Don't forget to move to the next chunk
			chunk_first = chunk_last;
		}

		tb.wait();

		// Compute the sum of the local values.
		T s{};
		for (auto &&i : sums)
		{
			s += i;
		}

		return s;
	}


}

