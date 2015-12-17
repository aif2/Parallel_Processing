
#pragma once

#include <thread>

#include "../../include/cs477.h"
#include "../../include/queue.h"
#include "../../include/data.h"
#include "../../include/net.h"
#include "../../include/http.h"
#include "../../include/file.h"

cs477::net::http_response make_response(int status, const std::string &json);

namespace async
{
	void run(const sockaddr_in &addr);
}

