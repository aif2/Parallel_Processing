
#pragma once

#include <atomic>



namespace cs477
{

#ifdef _WIN32
	using thread = HANDLE;
#else
	using thread = pthread_t;
#endif


	namespace details
	{

		// Base class for thread routine parameters.
		struct basic_thread_param
		{
			virtual ~basic_thread_param() { };

			// Derived classes must implement this method.
			virtual void operator()() const = 0;

			virtual void execute() const
			{
				try
				{
					// Call the derived class.
					(*this)();
				}
				catch (...)
				{
					// Just ignore any excpetions.
				}

				delete this;
			}
		};

		struct simple_thread_param : public basic_thread_param
		{
			simple_thread_param(void(*function)(void *), void *context)
				: function(function), context(context)
			{
			}

			virtual void operator()() const
			{
				if (function)
				{
					function(context);
				}
			}

			void(*function)(void *);
			void *context;
		};

		template <typename Fn>
		struct thread_param : public basic_thread_param
		{
			thread_param(Fn function)
				:function(std::move(function))
			{
			}

			virtual void operator()() const
			{
				function();
			}

			Fn function;
		};






#ifdef _WIN32

		inline DWORD __stdcall thread_start_routine(void *context)
		{
			auto param = static_cast<basic_thread_param *>(context);
			param->execute();
			return 0;
		}

#else

		inline void *thread_start_routine(void *context)
		{
			auto param = static_cast<basic_thread_param *>(context);
			param->execute();
			return nullptr;
		}

#endif

		inline thread create_thread(basic_thread_param *param)
		{
#ifdef _WIN32

			auto handle = CreateThread(nullptr, 0, thread_start_routine, param, 0, nullptr);
			if (!handle)
			{
				auto error = GetLastError();
				throw std::system_error(error, std::system_category());
			}

			return handle;

#else

			pthread_t tid;
			auto error = pthread_create(&tid, nullptr, thread_start_routine, param);
			if (error)
			{
				throw std::system_error(error, std::system_category());
			}
			return tid;

#endif

		}




	}

}