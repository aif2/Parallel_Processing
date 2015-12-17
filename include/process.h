
#pragma once


namespace cs477
{

	struct process_handle : public std::enable_shared_from_this<process_handle>
	{
	public:
		~process_handle()
		{
			if (th) CloseHandle(th);
			if (ph) CloseHandle(ph);
		}

		HANDLE ph;
		HANDLE th;

		auto wait()
		{
			struct param
			{
				std::shared_ptr<process_handle> proc;
				promise<int> prom;
			} *p = new param;
			p->proc = shared_from_this();

			auto w = CreateThreadpoolWait([](PTP_CALLBACK_INSTANCE, PVOID context, PTP_WAIT wait, TP_WAIT_RESULT result)
			{
				auto p = (param *)context;
				DWORD code;
				GetExitCodeProcess(p->proc->ph, &code);
				p->prom.set((int)code);
				delete p;
				CloseThreadpoolWait(wait);
			}, p, nullptr);

			SetThreadpoolWait(w, ph, nullptr);

			return p->prom.get_future();
		}
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