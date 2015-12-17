
#pragma once

#include <unordered_map>


namespace cs477
{

	namespace details
	{
		struct shm_handle
		{
			~shm_handle()
			{
				if (buf) UnmapViewOfFile(buf);
				if (fh) CloseHandle(fh);
			}

			HANDLE fh;
			void *buf;
			size_t len;
		};

		struct shm_manager
		{
			~shm_manager()
			{
				handles.clear();
			}

			std::shared_ptr<shm_handle> open(const std::string &name, size_t size)
			{
				lock_guard<> lock(mtx);

				auto pos = handles.find(name);
				if (pos != handles.end())
				{
					assert(pos->second->len == size);
					return pos->second;
				}
				else
				{
					auto mem = std::make_shared<shm_handle>();

					mem->fh = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(size), name.c_str());
					if (!mem->fh)
					{
						throw std::system_error(GetLastError(), std::system_category());
					}

					mem->len = size;
					mem->buf = MapViewOfFile(mem->fh, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<DWORD>(size));
					if (!mem->buf)
					{
						throw std::system_error(GetLastError(), std::system_category());
					}

					handles[name] = mem;

					return mem;
				}
			}

			void close(void *ptr)
			{
				lock_guard<> lock(mtx);

				for (auto i = handles.begin(); i != handles.end(); i++)
				{
					auto &kvp = *i;
					if (kvp.second->buf == ptr)
					{
						handles.erase(i);
						return;
					}
				}
			}

			mutex mtx;
			std::unordered_map<std::string, std::shared_ptr<shm_handle>> handles;
		};

		static shm_manager &get_shm_manager()
		{
			static shm_manager manager;
			return manager;
		}

	}




	inline void *shm_alloc(const std::string &name, size_t size)
	{
		auto &manager = details::get_shm_manager();
		auto mem = manager.open(name, size);
		return mem->buf;
	}

	inline void shm_free(void *ptr)
	{
		auto &manager = details::get_shm_manager();
		manager.close(ptr);
	}















	template <typename T, size_t N>
	class bounded_buffer
	{
	public:
		bounded_buffer(const std::string name)
		{

			_buf = static_cast<char *>(shm_alloc(name, N * sizeof(T) + sizeof(uint32_t)));
			_count = reinterpret_cast<uint32_t *>(_buf);
			_vector = reinterpret_cast<T *>(_count + 1);
	
			_lock.init(name + "-lock", 1, 1);
			_empty.init(name + "-empty", N, N);
			_full.init(name + "-full", 0, N);
		}

		~bounded_buffer()
		{
			shm_free(_buf);
		}

		void write(const T &value)
		{
			_empty.wait();
			_lock.wait();

			_vector[*_count] = value;
			(*_count)++;

			_lock.release();
			_full.release();
		}

		T read()
		{
			_full.wait();
			_lock.wait();

			auto t = _vector[*_count - 1];
			(*_count)--;

			_lock.release();
			_empty.release();

			return t;
		}

	private:
		char *_buf;
		uint32_t *_count;
		T *_vector;

		semaphore _empty;
		semaphore _full;
		semaphore _lock;
	};


	template <typename T, size_t N>
	class bounded_queue
	{
	public:
		bounded_queue(const std::string name)
		{

			_buf = static_cast<char *>(shm_alloc(name, N * sizeof(T) + 2 * sizeof(uint32_t)));
			_write = reinterpret_cast<uint32_t *>(_buf);
			_read = _write + 1;
			_vector = reinterpret_cast<T *>(_read + 1);

			_lock.init(name + "-lock", 1, 1);
			_empty.init(name + "-empty", N, N);
			_full.init(name + "-full", 0, N);
		}

		~bounded_queue()
		{
			shm_free(_buf);
		}

		void write(const T &value)
		{
			_empty.wait();
			_lock.wait();

			_vector[*_write] = value;
			(*_write)++;
			if (*_write == N) (*_write) = 0;

			_lock.release();
			_full.release();
		}

		T read()
		{
			_full.wait();
			_lock.wait();

			auto t = _vector[*_read];
			(*_read)++;
			if (*_read == N) (*_read) = 0;

			_lock.release();
			_empty.release();

			return t;
		}

	private:
		char *_buf;
		uint32_t *_read;
		uint32_t *_write;
		T *_vector;

		semaphore _empty;
		semaphore _full;
		semaphore _lock;


	};

}