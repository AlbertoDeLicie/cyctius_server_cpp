#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <thread>
#include <vector>
#include "router_coro.h"

//class HttpSessionCoro;

class HttpServerCoro {
public:
	HttpServerCoro(unsigned short port, unsigned int thread_count);
	~HttpServerCoro();

	std::shared_ptr<RouterCoro> get_router();

	void run();

private:
	boost::asio::awaitable<void> accept();
	boost::asio::awaitable<void> clean_expired_sessions();

	void terminate_session(size_t i) const;

	unsigned int m_thread_count;
	boost::asio::io_context m_io_context;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::shared_ptr<RouterCoro> m_router;

	// лишь ссылаемся на сессию но не продлеваем время ее жизни, потому-что сессия может быть закрыта извне
	//std::vector<std::weak_ptr<HttpSessionCoro>> m_sessions;
};