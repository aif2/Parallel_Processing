
#pragma once

#include "cs477.h"


namespace cs477
{

	class mutex
	{
	public:
		mutex();

		mutex(const mutex &) = delete;
		mutex &operator =(const mutex &) = delete;

		mutex(mutex &&) = delete;
		mutex &operator =(mutex &&) = delete;

		~mutex();

	public:
		void lock();
		void unlock();

	private:
#ifdef _WIN32
		CRITICAL_SECTION cs;
#else
		pthread_mutex_t mtx;
#endif
		friend class condition_variable;
	};




	template <typename Lock = mutex>
	class lock_guard
	{
	public:
		lock_guard(Lock &lock);
		~lock_guard();

		lock_guard(const lock_guard &) = delete;
		lock_guard &operator=(const lock_guard &) = delete;

	private:
		Lock &my_lock;
	};




	template <typename Fn>
	auto lock(mutex &mtx, Fn fn)
	{
		mtx.lock();
		try
		{
			fn();
			mtx.unlock();
			return t;
		}
		catch (...)
		{
			mtx.unlock();
			throw;
		}
	}



	class condition_variable
	{
	public:
		condition_variable();

		condition_variable(const condition_variable &) = delete;
		condition_variable &operator =(const condition_variable &) = delete;

		condition_variable(condition_variable &&) = delete;
		condition_variable &operator =(condition_variable &&) = delete;

		~condition_variable();

	public:
		void notify_one();
		void notify_all();

		void wait(mutex &mtx);
		bool wait(mutex &mtx, std::chrono::milliseconds ms);

	private:
#ifdef _WIN32
		CONDITION_VARIABLE cv;
#else
		pthread_mutex_t cv;
#endif

	};



	template <typename T> class future;


	class semaphore
	{
	public:
		semaphore();

		semaphore(const semaphore &) = delete;
		semaphore &operator =(const semaphore &) = delete;

		semaphore(semaphore &&) = delete;
		semaphore &operator =(semaphore &&) = delete;

		~semaphore();

	public:
		void init(const std::string &name, size_t value, size_t count);
		void init(const std::string &name);

	public:
		void wait();
		void release();

		future<void> wait_async();

	public:
		void lock() { wait(); }
		void unlock() { release(); }

	private:
		HANDLE sem;
		std::atomic<int> async;

		static void __stdcall wait_callback(PTP_CALLBACK_INSTANCE, PVOID context, PTP_WAIT wait, TP_WAIT_RESULT result);
	};




}