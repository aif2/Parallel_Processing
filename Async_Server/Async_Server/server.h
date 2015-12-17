
#pragma once

#include <thread>

#include "../../include/cs477.h"
#include "../../include/queue.h"
#include "../../include/data.h"
#include "../../include/net.h"
#include "../../include/http.h"
#include "../../include/file.h"


cs477::data::statement make_query(cs477::data::database &db, const cs477::net::http_request &request);
cs477::data::statement make_insert(cs477::data::database &db, const cs477::net::http_request &request);
cs477::data::statement make_delete(cs477::data::database &db, const cs477::net::http_request &request);

cs477::net::http_response make_response(int status, const std::string &json);

namespace single_thread
{

	void run(const sockaddr_in &addr, std::shared_ptr<cs477::data::database> db);
}

namespace multi_thread
{
	void run(std::shared_ptr<cs477::data::database> db);
	void run2(std::shared_ptr<cs477::data::database> db);
}

namespace async
{
	void run(const sockaddr_in &addr, std::shared_ptr<cs477::data::database> db);
}

