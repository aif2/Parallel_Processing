
#pragma once

#include "cs477.h"


namespace cs477
{

	namespace net
	{

		class http_request
		{
		public:
			std::string method;
			std::string url;
			std::vector<std::pair<std::string, std::string>> headers;
			std::string body;
		};

		http_request read_http_request(const char *buf, uint32_t len);
		http_request read_http_request(socket &sock);
		future<http_request> read_http_request_async(socket sock);

		class http_response
		{
		public:
			int status;
			std::string message;
			std::vector<std::pair<std::string, std::string>> headers;
			std::string body;
		};

		std::string write_http_response(const http_response &rsp);
		void write_http_response(socket &sock, const http_response &rsp);
		future<void> write_http_response_async(socket sock, const http_response &rsp);


		const std::error_category &http_category();
	}

}

#include "http.inl"
