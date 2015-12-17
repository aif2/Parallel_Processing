
#include "../http_parser/http_parser.h"
#include "http.h"
#pragma comment(lib, "http_parser.lib")


namespace cs477
{

	namespace net
	{

		struct http_request_parse_state : public http_parser, public http_parser_settings
		{
			http_request rq;

			enum
			{
				url,
				name,
				value,
				body,
				done,
			} state;
		};

		inline int on_request_begin_message(http_parser *parser)
		{
			auto state = (http_request_parse_state *)parser->data;
			state->rq.method = http_method_str(http_method(parser->method));
			return 0;
		}

		inline int on_request_url(http_parser *parser, const char *at, size_t length)
		{
			auto state = (http_request_parse_state *)parser->data;
			state->rq.url.append(at, length);
			state->state = http_request_parse_state::url;
			return 0;
		}

		inline int on_request_header_field(http_parser *parser, const char *at, size_t length)
		{
			auto state = (http_request_parse_state *)parser->data;
			if (state->state != http_request_parse_state::name)
			{
				state->rq.headers.emplace_back();
				state->state = http_request_parse_state::name;
			}
			state->rq.headers.back().first.append(at, length);
			return 0;
		}

		inline int on_request_header_value(http_parser *parser, const char *at, size_t length)
		{
			auto state = (http_request_parse_state *)parser->data;
			state->rq.headers.back().second.append(at, length);
			state->state = http_request_parse_state::value;
			return 0;
		}

		inline int on_request_headers_complete(http_parser *parser)
		{
			auto state = (http_request_parse_state *)parser->data;
			state->state = http_request_parse_state::body;
			return 0;
		}

		inline int on_request_body(http_parser *parser, const char *at, size_t length)
		{
			auto state = (http_request_parse_state *)parser->data;
			state->rq.body.append(at, length);
			return 0;
		}

		inline int on_request_message_complete(http_parser *parser)
		{
			auto state = (http_request_parse_state *)parser->data;
			state->state = http_request_parse_state::done;
			return 0;
		}

		inline http_request read_http_request(const char *str, uint32_t len)
		{
			http_request_parse_state state;

			state.on_message_begin = on_request_begin_message;
			state.on_url = on_request_url;
			state.on_header_field = on_request_header_field;
			state.on_header_value = on_request_header_value;
			state.on_headers_complete = on_request_headers_complete;
			state.on_body = on_request_body;
			state.on_message_complete = on_request_message_complete;

			http_parser_init(&state, HTTP_REQUEST);
			state.data = &state;

			auto parsed = http_parser_execute(&state, &state, str, len);
			if (state.http_errno)
			{
				throw std::system_error(state.http_errno, http_category());
			}
			else if (state.state != http_request_parse_state::done)
			{
				throw std::exception("Invalid state");
			}

			return std::move(state.rq);
		}

		inline http_request read_http_request(socket &sock)
		{
			http_request_parse_state state;

			state.on_message_begin = on_request_begin_message;
			state.on_url = on_request_url;
			state.on_header_field = on_request_header_field;
			state.on_header_value = on_request_header_value;
			state.on_headers_complete = on_request_headers_complete;
			state.on_body = on_request_body;
			state.on_message_complete = on_request_message_complete;

			http_parser_init(&state, HTTP_REQUEST);
			state.data = &state;

			std::string str;
			str.resize(65536);

			for (;;)
			{
				auto len = sock.recv(&str.front(), 65536);
				auto parsed = http_parser_execute(&state, &state, str.c_str(), len);
				if (parsed != len)
				{
					throw std::system_error(state.http_errno, http_category());
				}

				if (state.state == http_request_parse_state::done)
				{
					break;
				}
			}

			return std::move(state.rq);
		}

		inline future<http_request> read_http_request_async(socket sock)
		{
			auto state = std::make_shared<http_request_parse_state>();

			state->on_message_begin = on_request_begin_message;
			state->on_url = on_request_url;
			state->on_header_field = on_request_header_field;
			state->on_header_value = on_request_header_value;
			state->on_headers_complete = on_request_headers_complete;
			state->on_body = on_request_body;
			state->on_message_complete = on_request_message_complete;

			http_parser_init(state.get(), HTTP_REQUEST);
			state->data = state.get();

			return do_while([state, sock] () mutable
			{
				return sock.recv_async().then([state] (auto f) mutable
				{
					auto str = f.get();
					auto parsed = http_parser_execute(state.get(), state.get(), str.c_str(), str.length());
					if (parsed != str.length())
					{
						throw std::system_error(state->http_errno, http_category());
					}

					return state->state != http_request_parse_state::done;
				});
			}).then([state, sock] (future<void> f)
			{
				return std::move(state->rq);
			});

		}

		inline std::string write_http_response(const http_response &rsp)
		{
			char line[128];

			std::string text;

			sprintf_s(line, "HTTP/1.1 %d %s\r\n", rsp.status, rsp.message.c_str());
			text.append(line);

			if (rsp.body.length())
			{
				sprintf_s(line, "Content-Length : %d\r\n", static_cast<uint32_t>(rsp.body.length()));
				text.append(line);
			}

			for (auto &hdr : rsp.headers)
			{
				sprintf_s(line, "%s : %s\r\n", hdr.first.c_str(), hdr.second.c_str());
				text.append(line);
			}

			text.append("\r\n");

			text.append(rsp.body);
			return text;
		}

		inline void write_http_response(socket &sock, const http_response &rsp)
		{
			auto text = write_http_response(rsp);
			sock.send(text.c_str(), static_cast<uint32_t>(text.length()));
		}

		inline future<void> write_http_response_async(socket sock, const http_response &rsp)
		{
			auto text = write_http_response(rsp);
			return sock.send_async(text.c_str(), static_cast<uint32_t>(text.length()));
		}



		class _http_error_category : public std::error_category
		{
			virtual const char *name() const _NOEXCEPT
			{
				return "http";
			}

			virtual std::string message(int code) const
			{
				return http_errno_description(static_cast<http_errno>(code));
			}
		};


		inline const std::error_category & http_category()
		{
			static _http_error_category *cat = new _http_error_category;
			return *cat;
		}




	}
}