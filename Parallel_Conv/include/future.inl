
#pragma once


namespace cs477
{

	namespace details
	{

		inline basic_shared_state::basic_shared_state()
			: state(future_state::not_ready)
		{
		}

		inline basic_shared_state::~basic_shared_state()
		{
			if (ex)
			{
				try
				{
					std::rethrow_exception(ex);
				}
				catch (std::exception &ex)
				{
					printf("Warning: uncaught exception! %s\n", ex.what());
				}
				catch (...)
				{
					printf("Warning: uncaught exception!\n");
				}
			}
		}

		inline void basic_shared_state::set_exception(std::exception_ptr ptr)
		{
			lock_guard<> guard(mtx);
			if (state != future_state::not_ready)
			{
				// TODO: Throw
			}

			state = future_state::has_error;
			ex = std::move(ptr);

			if (next)
			{
				next->execute();
				next.reset();
			}

			cv.notify_all();
		}

		inline void basic_shared_state::wait()
		{
			lock_guard<> guard(mtx);
			while (state == future_state::not_ready)
			{
				cv.wait(mtx);
			}

			if (ex)
			{
				auto ptr = std::move(ex);
				std::rethrow_exception(ptr);
			}
		}

	
		template <typename T>
		void shared_state<T>::set(T val)
		{
			lock_guard<> guard(mtx);
			if (state != future_state::not_ready)
			{
				// TODO: Throw
			}

			value = std::move(val);
			state = future_state::has_value;

			if (next)
			{
				next->execute();
				next.reset();
			}

			cv.notify_all();
		}

		template <typename T>
		T shared_state<T>::get()
		{
			wait();
			if (ex)
			{
				auto ptr = std::move(ex);
				std::rethrow_exception(ptr);
			}

			return std::move(value);
		}



		inline void shared_state<void>::set()
		{
			lock_guard<> guard(mtx);
			if (state != future_state::not_ready)
			{
				// TODO: Throw
			}

			state = future_state::has_value;

			if (next)
			{
				next->execute();
				next.reset();
			}

			cv.notify_all();
		}

		inline void shared_state<void>::get()
		{
			wait();
			if (ex)
			{
				auto ptr = std::move(ex);
				std::rethrow_exception(ptr);
			}
		}




	}



	template <typename T> 
	future<T>::future(future<future<T>> fft)
	{
		promise<T> p;
		*this = p.get_future();

		fft.then([p = std::move(p)](future<future<T>> fft) mutable
		{
			try
			{
				future<T> ft = fft.get();
				ft.then([p = std::move(p)](future<T> ft) mutable
				{
					try
					{
						auto t = ft.get();
						p.set(t);
					}
					catch (...)
					{
						p.set_exception(std::current_exception());
					}
				});
			}
			catch (...)
			{
				p.set_exception(std::current_exception());
			}
		});
	}

	future<void>::future(future<future<void>> ffv)
	{
		promise<void> p;
		*this = p.get_future();

		ffv.then([p = std::move(p)](future<future<void>> ffv) mutable
		{
			try
			{
				future<void> fv = ffv.get();
				fv.then([p = std::move(p)](future<void> fv) mutable
				{
					try
					{
						fv.get();
						p.set();
					}
					catch (...)
					{
						p.set_exception(std::current_exception());
					}
				});
			}
			catch (...)
			{
				p.set_exception(std::current_exception());
			}
		});
	}


}