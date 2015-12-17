
#pragma once


namespace cs477
{

	inline thread create_thread(void(*function)(void *), void *context)
	{
		auto param = new details::simple_thread_param(function, context);
		return details::create_thread(param);
	}

	template <typename Fn> thread create_thread(Fn fn)
	{
		auto param = new details::thread_param<Fn>(std::move(fn));
		return details::create_thread(param);
	}

	inline void join(thread thread)
	{
#ifdef _WIN32

		auto result = WaitForSingleObject(thread, INFINITE);
		if (result != WAIT_OBJECT_0)
		{
			auto error = GetLastError();
			throw std::system_error(error, std::generic_category());
		}

		CloseHandle(thread);

#else

		auto error = pthread_join(thread, nullptr);
		if (error)
		{
			throw std::system_error(error, std::generic_category());
		}

#endif
	}



}
