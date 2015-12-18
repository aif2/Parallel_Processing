
#include "stdafx.h"
#include "server.h"


namespace async
{

	std::string base_dir = "C:\\server\\";
	void socket_handler(cs477::net::socket sock)
	{

		// Read an http request
		auto f = cs477::net::read_http_request_async(sock).then([sock](auto f)
		{
			auto rq = f.get();

			std::string file_path = rq.url;
			file_path = base_dir + file_path;

				try {
					cs477::read_file_async(file_path.c_str()).then([sock](auto f) {
					std::string s = f.get();
					cs477::net::write_http_response_async(sock, make_response(200, s));

				});
			}
			catch (...) {

				cs477::net::write_http_response_async(sock, make_response(404, "File not found"));
			}
			return 0;

		});
	}


	void run(const sockaddr_in &addr, std::shared_ptr<cs477::data::database> db)
	{
		auto host = std::make_shared<cs477::net::acceptor>();
		host->listen(addr);

		for (int i = 0; i < 32; i++)
		{
			host->accept_async(socket_handler);
		}

		// Just wait forever.
		cs477::promise<void> p;
		auto f = p.get_future();
		f.wait();
	}
}
