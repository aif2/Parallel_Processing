
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
		bounded_buffer()
		{
			_buf = nullptr;
			_count = nullptr;
			_vector = nullptr;
		}

		~bounded_buffer()
		{
			shm_free(_buf);
		}

		void create(const std::string name)
		{

			_buf = static_cast<char *>(shm_alloc(name, N * sizeof(T) + sizeof(uint32_t)));
			_count = reinterpret_cast<uint32_t *>(_buf);
			_vector = reinterpret_cast<T *>(_count + 1);

			_lock.init(name + "-lock", 1, 1);
			_empty.init(name + "-empty", N, N);
			_full.init(name + "-full", 0, N);
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
		bounded_queue()
		{
			_buf = nullptr;
			_write = nullptr;
			_read = nullptr;
			_vector = nullptr;
		}

		~bounded_queue()
		{
			shm_free(_buf);
		}

	public:
		const std::string &name() const
		{
			return _name;
		}

		void create(const std::string name)
		{
			_name = name;
			_buf = static_cast<char *>(shm_alloc(name, N * sizeof(T) + 2 * sizeof(uint32_t)));
			_write = reinterpret_cast<uint32_t *>(_buf);
			_read = _write + 1;
			_vector = reinterpret_cast<T *>(_read + 1);

			_lock.init(name + "-lock", 1, 1);
			_empty.init(name + "-empty", N, N);
			_full.init(name + "-full", 0, N);
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

		future<void> write_async(T value)
		{
			future<void> f = _empty.wait_async();
			f = f.then([this, value](auto f) mutable
			{
				f.get();
				f = _lock.wait_async();
				f = f.then([this, value](auto f)
				{
					f.get();

					_vector[*_write] = value;
					(*_write)++;
					if (*_write == N) (*_write) = 0;

					_lock.release();
					_full.release();
				});
				return f;
			});
			
			return f;
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

		future<T> read_async()
		{
			future<T> f = _full.wait_async().then([this](auto f) mutable
			{
				f.get();
				f = _lock.wait_async();
				return f.then([this](auto f)
				{
					f.get();

					auto t = _vector[*_read];
					(*_read)++;
					if (*_read == N) (*_read) = 0;

					_lock.release();
					_empty.release();

					return t;
				});
			});

			return f;
		}

	private:
		std::string _name;

		char *_buf;
		uint32_t *_read;
		uint32_t *_write;
		T *_vector;

		semaphore _empty;
		semaphore _full;
		semaphore _lock;

	};










	class shm_pool
	{
	public:
		struct pointer
		{
			pointer()
			{
				index = 0;
				count = 0;
			}

			bool operator ==(nullptr_t) const
			{
				return count == 0;
			}

			bool operator !=(nullptr_t) const
			{
				return count != 0;
			}

			uint32_t index;
			uint32_t count;
		};

	public:
		shm_pool()
		{
			_pos = nullptr;
		}

		~shm_pool()
		{
			if (_pos)
			{
				shm_free(_pos);
			}
		}

	public:

		const std::string &name() const
		{
			return _name;
		}

		void create(const std::string name, uint32_t block_size, uint32_t num_blocks)
		{
			_name = name;

			_block_size = block_size;
			_count = num_blocks;
	
			// header is 1 byte per block + 4 bytes for pos.
			uint32_t header_size = num_blocks + sizeof(uint32_t);
			auto header_blocks = get_block_count(header_size);

			_pos = (uint32_t *)shm_alloc(name, (header_blocks + num_blocks) * block_size);
			_header = (char *)(_pos + 1);
			_first_block = (char *)_pos + (header_blocks * block_size);
		}

		pointer allocate(uint32_t len)
		{
			auto blocks = get_block_count(len);

			// Find the first free block
			auto init = _header + *_pos;
			auto begin = init;
			auto end = _header + _count;

			// Search from init to end for blocks # of 0's
			auto pos = std::search_n(begin, end, static_cast<ptrdiff_t>(blocks), 0);
			if (pos == end)
			{
				// Now search from the front of the header to init
				begin = _header;
				end = init;
				pos = std::search_n(begin, init, static_cast<ptrdiff_t>(blocks), 0);
				if (pos == init)
				{
					throw std::bad_alloc();
				}
			}

			// Mark the blocks as used.
			mark(pos, blocks, 1);

			// Move the pointer to the end of the blocks
			*_pos = static_cast<uint32_t>((pos + blocks) - _header);

			// Return the pointer
			pointer p;
			p.index = static_cast<uint32_t>(pos - _header);
			p.count = blocks;
			return p;
		}

		void deallocate(const pointer &ptr)
		{
			auto pos = _header + ptr.index;
			mark(pos, ptr.count, 0);
		}

		char *operator()(const pointer &ptr)
		{
			return _first_block + (ptr.index * _block_size);
		}

	private:
		uint32_t get_block_count(uint32_t bytes)
		{
			auto blocks = (bytes / _block_size) + 1;
			if (bytes % _block_size == 0)
			{
				blocks--;
			}
			return blocks;
		}

		void mark(char *pos, uint32_t count, char value)
		{
			auto end = pos + count;
			while (pos < end)
			{
				*pos++ = value;
			}
		}

	private:
		std::string _name;

		uint32_t _block_size;
		uint32_t _count;
		
		uint32_t *_pos;			// pointer to shared first_free page.
		char *_header;			// byte[] of usage indicators for pages.
		char *_first_block;		// pointer to first block of shared memory
	};



}