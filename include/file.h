
#pragma once

#include "cs477.h"


namespace cs477
{

	inline std::string read_file(const char *path)
	{
		HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			throw std::system_error(GetLastError(), std::system_category());
		}

		auto size = GetFileSize(file, nullptr);

		std::string str;
		str.resize(size);

		DWORD read = 0;
		if (!ReadFile(file, &str.front(), size, &read, nullptr))
		{
			throw std::system_error(GetLastError(), std::system_category());
		}

		return str;
	}

	inline void write_file(const char *path, const std::string &str)
	{
		HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			throw std::system_error(GetLastError(), std::system_category());
		}
		DWORD wrote = 0;
		if (!WriteFile(file, str.c_str(), (DWORD)str.c_str(), &wrote, nullptr))
		{
			throw std::system_error(GetLastError(), std::system_category());
		}
	}


	inline future<std::string> read_file_async(const char *path)
	{
		HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			throw std::system_error(GetLastError(), std::system_category());
		}

		struct param_t
		{
			OVERLAPPED ol;
			std::string str;
			promise<std::string> p;
		};

		auto param = new param_t;
		memset(&param->ol, 0, sizeof(OVERLAPPED));

		auto size = GetFileSize(file, nullptr);
		param->str.resize(size);

		auto f = param->p.get_future();

		auto io = CreateThreadpoolIo(file, [](PTP_CALLBACK_INSTANCE, PVOID, PVOID Overlapped, ULONG IoResult, ULONG_PTR, PTP_IO Io)
		{
			auto param = (param_t *)Overlapped;

			if (IoResult)
			{
				param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
			}
			else
			{
				param->p.set(std::move(param->str));
			}

			delete param;
			CloseThreadpoolIo(Io);
		}, nullptr, nullptr);
		
		if (!io)
		{
			param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
			return f;
		}

		StartThreadpoolIo(io);

		if (!ReadFile(file, &param->str.front(), size, nullptr, &param->ol))
		{
			auto err = GetLastError();
			if (err != ERROR_IO_PENDING)
			{
				CancelThreadpoolIo(io);
				CloseThreadpoolIo(io);
				param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
			}
		}

		return f;
	}

	inline future<void> write_file_async(const char *path, std::string str)
	{
		HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			throw std::system_error(GetLastError(), std::system_category());
		}

		struct param_t
		{
			OVERLAPPED ol;
			std::string str;
			promise<void> p;
		};

		auto param = new param_t;
		memset(&param->ol, 0, sizeof(OVERLAPPED));

		param->str = std::move(str);

		auto f = param->p.get_future();

		auto io = CreateThreadpoolIo(file, [](PTP_CALLBACK_INSTANCE, PVOID, PVOID Overlapped, ULONG IoResult, ULONG_PTR, PTP_IO Io)
		{
			auto param = (param_t *)Overlapped;

			if (IoResult)
			{
				param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
			}
			else
			{
				param->p.set();
			}

			delete param;
			CloseThreadpoolIo(Io);
		}, nullptr, nullptr);

		if (!io)
		{
			param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
			return f;
		}

		StartThreadpoolIo(io);

		if (!WriteFile(file, param->str.c_str(), (DWORD)param->str.length(), nullptr, &param->ol))
		{
			auto err = GetLastError();
			if (err != ERROR_IO_PENDING)
			{
				CancelThreadpoolIo(io);
				CloseThreadpoolIo(io);
				param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
			}
		}

		return f;
	}

}