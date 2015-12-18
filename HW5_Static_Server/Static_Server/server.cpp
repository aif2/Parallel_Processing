// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "server.h"

int main()
{
	cs477::net::initialize();
	
	auto addr = cs477::net::resolve_address("localhost", 8080);

	async::run(addr);

	cs477::net::finalize();
	return 0;
}


cs477::net::http_response make_response(int status, const std::string &json)
{
	cs477::net::http_response rsp;
	rsp.status = status;
	switch (status)
	{
	case 200:
		rsp.message = "Ok";
		break;
	case 404:
		rsp.message = "Not Found";
		break;
	case 500:
		rsp.message = "Internal Error";
		break;
	}

	if (status == 200)
	{
		if (json.length())
		{
			rsp.body = json;
			rsp.headers.emplace_back("Content-Type", "application/json");
		}
	}

	return rsp;
}

