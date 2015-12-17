
#pragma once

#include "cs477.h"

namespace cs477
{
	namespace net
	{
		class socket;
		class acceptor;
	}

	namespace details
	{
		class socket;

		struct overlapped
		{
			enum io_type
			{
				accept,
				recv,
				send,
			};

			OVERLAPPED ol;
			io_type type;
		};

		class async_accept
		{
		public:
			async_accept(std::function<void(net::socket)> fn)
				: fn(std::move(fn))
			{
				memset(&ol.ol, 0, sizeof(OVERLAPPED));
				ol.type = overlapped::accept;
			}
			
			void call_handler();
		
			overlapped ol;
			std::function<void(net::socket)> fn;
			socket *sock;
			char buf[2 * (sizeof(sockaddr_in) + 16)];
		};

		class async_send
		{
		public:
			async_send()
			{
				memset(&ol.ol, 0, sizeof(OVERLAPPED));
				ol.type = overlapped::send;
			}

			overlapped ol;
			std::string buf;
			promise<void> promise;
		};

		class async_recv 
		{
		public:
			async_recv(size_t count)
			{
				memset(&ol.ol, 0, sizeof(OVERLAPPED));
				ol.type = overlapped::recv;
				buf.resize(count);
			}

			overlapped ol;
			std::string buf;
			promise<std::string> promise;
		};

		class socket
		{
		public:
			socket()
				: ref(1), handle(INVALID_SOCKET), io(nullptr)
			{
			}

			~socket()
			{
				if (handle != INVALID_SOCKET)
				{
					closesocket(handle);
				}
				if (io)
				{
					CloseThreadpoolIo(io);
				}
			}

		public:
			void addref()
			{
				++ref;
			}

			void release()
			{
				if (ref == 1)
				{
					delete this;
					return;
				}
				--ref;
			}

		public:
			void create(SOCKET sock)
			{
				if (sock == INVALID_SOCKET)
				{
					handle = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

					GUID guid = WSAID_ACCEPTEX;
					DWORD bytes = 0;
					WSAIoctl(handle, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), &fn_accept, sizeof(fn_accept), &bytes, nullptr, nullptr);
				}
				else
				{
					handle = sock;
				}

				io = CreateThreadpoolIo(reinterpret_cast<HANDLE>(handle), io_callback, this, nullptr);
			}

			template <typename Fn>
			void socket::accept(Fn fn)
			{
				auto op = new async_accept(std::move(fn));
				accept(op);
			}

		private:
			void socket::accept(async_accept *op)
			{
				op->sock = new socket();
				op->sock->create(INVALID_SOCKET);

				memset(&op->ol.ol, 0, sizeof(OVERLAPPED));
				addref();
				StartThreadpoolIo(io);

				if (!fn_accept(handle, op->sock->handle, op->buf, 0, (sizeof(sockaddr_in) + 16), (sizeof(sockaddr_in) + 16), nullptr, &op->ol.ol))
				{
					auto err = GetLastError();
					if (err != ERROR_IO_PENDING)
					{
						release();
						CancelThreadpoolIo(io);
						delete op;
						throw std::system_error(err, std::system_category());
					}
				}
			}

		public:
			future<void> send(const char *buf, size_t len)
			{
				auto op = new async_send();
				op->buf = { buf, len };
				auto f = op->promise.get_future();

				addref();
				StartThreadpoolIo(io);

				WSABUF wsabuf;
				wsabuf.buf = const_cast<char *>(buf);
				wsabuf.len = len;
				auto result = WSASend(handle, &wsabuf, 1, nullptr, 0, &op->ol.ol, nullptr);
				if (result == SOCKET_ERROR)
				{
					auto err = GetLastError();
					if (err != ERROR_IO_PENDING)
					{
						release();
						CancelThreadpoolIo(io);
						delete op;
						throw std::system_error(err, std::system_category());
					}
				}

				return f;
			}

			future<std::string> recv(size_t len)
			{
				auto op = new async_recv(len);

				auto f = op->promise.get_future();

				addref();
				StartThreadpoolIo(io);

				WSABUF wsabuf;
				wsabuf.buf = const_cast<char *>(op->buf.c_str());
				wsabuf.len = len;
				DWORD flags = 0;
				auto result = WSARecv(handle, &wsabuf, 1, nullptr, &flags, &op->ol.ol, nullptr);
				if (result == SOCKET_ERROR)
				{
					auto err = GetLastError();
					if (err != ERROR_IO_PENDING)
					{
						release();
						CancelThreadpoolIo(io);
						delete op;
						throw std::system_error(err, std::system_category());
					}
				}

				return f;
			}

		private:
			static void __stdcall io_callback(PTP_CALLBACK_INSTANCE, PVOID Context, PVOID Overlapped, ULONG IoResult, ULONG_PTR NumberOfBytesTransferred, PTP_IO)
			{
				auto ol = (overlapped *)Overlapped;
				auto sock = (socket *)Context;

				if (ol->type == overlapped::recv)
				{
					auto recv = reinterpret_cast<async_recv *>(ol);
					if (IoResult || !NumberOfBytesTransferred)
					{
						recv->promise.set_exception(std::make_exception_ptr(std::exception{}));
					}
					recv->buf.resize(NumberOfBytesTransferred);
					recv->promise.set(std::move(recv->buf));
					delete recv;
				}
				else if (ol->type == overlapped::send)
				{
					auto send = reinterpret_cast<async_send *>(ol);
					if (IoResult || !NumberOfBytesTransferred)
					{
						send->promise.set_exception(std::make_exception_ptr(std::exception{}));
					}
					send->promise.set();
					delete send;
				}
				else if (ol->type == overlapped::accept)
				{
					auto accept = reinterpret_cast<async_accept *>(ol);
					if (!IoResult)
					{
						auto err = setsockopt(accept->sock->handle, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&sock->handle, sizeof(SOCKET));
						if (err == SOCKET_ERROR)
						{
							IoResult = GetLastError();
						}
					}
					if (!IoResult)
					{
						try
						{
							accept->call_handler();
							// Queue another accept.
							sock->accept(accept);
						}
						catch (...)
						{
							delete accept;
						}
					}
					else
					{
						delete accept;
					}
				}
				else
				{
					// ?
				}

				sock->release();
			}

		public:
			std::atomic<int> ref;

			SOCKET handle;
			PTP_IO io;

			LPFN_ACCEPTEX fn_accept;
		};

	}


	namespace net
	{

		void initialize();
		void finalize();

		sockaddr_in resolve_address(const std::string &hostname, int port);


		class socket
		{
		public:
			socket();
			~socket();

			socket(socket &&sock);
			socket &operator=(socket &&sock);

			socket(const socket &sock);
			socket &operator=(const socket &sock);

		public:
			void connect(const sockaddr_in &addr);

			void send(const char *buf, size_t len);
			size_t recv(char *buf, size_t len);

		public:
			future<void> send_async(const char *buf, size_t len);
			future<std::string> recv_async();

		private:
			details::socket *sock;
			friend class acceptor;
			friend class details::async_accept;
		};


		class acceptor
		{
		public:
			acceptor();
			~acceptor();

			acceptor(acceptor &&) = delete;
			acceptor &operator=(acceptor &&) = delete;

			acceptor(const acceptor &) = delete;
			acceptor &operator=(const acceptor &sock) = delete;

		public:
			void listen(const sockaddr_in &in);
			socket accept();
			template <typename Fn> void accept_async(Fn fn);

		private:
			details::socket *sock;
		};


	}


}


#include "net.inl"
