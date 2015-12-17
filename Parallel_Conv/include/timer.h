
#pragma once

#include <chrono>
#include <tuple>


inline auto now()
{
	return std::chrono::high_resolution_clock::now();
}

inline auto to_seconds(
	std::chrono::time_point<std::chrono::high_resolution_clock> t1, 
	std::chrono::time_point<std::chrono::high_resolution_clock> t2)
{
	std::chrono::duration<double> diff = t2 - t1;
	return diff.count();
}


#ifdef _WIN32

inline std::tuple<int64_t, int64_t, int64_t> time_point()
{
	union time
	{
		FILETIME ft;
		LARGE_INTEGER li;
	};
	time c, e, k, u;
	GetProcessTimes(GetCurrentProcess(), &c.ft, &e.ft, &k.ft, &u.ft);

	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);

	return std::make_tuple(t.QuadPart, k.li.QuadPart, u.li.QuadPart);
}


inline auto diff(const std::tuple<int64_t, int64_t, int64_t> &tp1, const std::tuple<int64_t, int64_t, int64_t> &tp2)
{
	auto dt = std::get<0>(tp2) - std::get<0>(tp1);
	auto dk = std::get<1>(tp2) - std::get<1>(tp1);
	auto du = std::get<2>(tp2) - std::get<2>(tp1);

	LARGE_INTEGER f;
	QueryPerformanceFrequency(&f);

	return std::make_tuple(
		static_cast<double>(dt) / static_cast<double>(f.QuadPart),
		static_cast<double>(dk) / 10000000.0,
		static_cast<double>(du) / 10000000.0);
}

#endif


