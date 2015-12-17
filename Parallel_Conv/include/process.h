
#pragma once


namespace cs477
{

	struct process_handle
	{
	public:
		~process_handle()
		{
			if (th) CloseHandle(th);
			if (ph) CloseHandle(ph);
		}

		HANDLE ph;
		HANDLE th;
	};

	using process = std::shared_ptr<process_handle>;


	inline process create_process(const std::string &path, const std::string &args)
	{
		STARTUPINFOA si = { sizeof(STARTUPINFO), 0 };
		PROCESS_INFORMATION pi = { 0 };
		if (!CreateProcessA(path.c_str(), const_cast<char *>(args.c_str()), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
		{
			throw std::system_error(GetLastError(), std::system_category());
		}

		auto p = std::make_shared<process_handle>();
		p->ph = pi.hProcess;
		p->th = pi.hThread;

		return p;
	}

	

}