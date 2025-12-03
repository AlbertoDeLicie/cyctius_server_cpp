#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <thread>
#include <vector>
#include "http_router_coro.h"
#include "websocket_router.h"

//class HttpSessionCoro;

class HttpServerCoro {
public:
	HttpServerCoro(unsigned short port, unsigned int thread_count);
	~HttpServerCoro();

	std::shared_ptr<HttpRouterCoro> get_http_router();
	std::shared_ptr<WebsocketRouter> get_websocket_router();

	void run();
	boost::asio::io_context& context();

private:
	boost::asio::awaitable<void> accept();

	unsigned int m_thread_count;
	boost::asio::io_context m_io_context;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::shared_ptr<HttpRouterCoro> m_http_router;
	std::shared_ptr<WebsocketRouter> m_websocket_router;
};