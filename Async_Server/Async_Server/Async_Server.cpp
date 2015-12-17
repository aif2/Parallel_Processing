// Async_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../include/cs477.h"
#include "server.h"

namespace cs477 {

	namespace async {

		const std::string base_dir = "C:\\Server";
		void socket_handler(cs477::net::socket sock) {

			// Read an http request
			auto f = cs477::net::read_http_request_async(sock).then([sock](auto f) {
			auto rq = f.get();
			auto file_path = rq.url;

				try {
					cs477::read_file_async(file_path.c_str()).then([sock](auto f) {
						auto s = f.get();
						auto r = make_response(200, s);
						cs477::net::write_http_response_async(sock, r);
					});
				}
				catch (...) {
					auto r = make_response(404, {});
					cs477::net::write_http_response_async(sock, r);
				}

				return 0;
			});
		}


		void run(const sockaddr_in &addr) {
			auto host = std::make_shared<cs477::net::acceptor>();
			host->listen(addr);

			for (int i = 0; i < 32; i++) {
				host->accept_async(socket_handler);
			}

			// Just wait forever.
			cs477::promise<void> p;
			auto f = p.get_future();
			f.wait();
		}
	}
}