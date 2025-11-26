#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <thread>
#include <vector>
#include "router.h"

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

class HttpServer {
public:
	HttpServer(unsigned short port, unsigned int thread_count);
	~HttpServer();
	void set_router(std::shared_ptr<Router> router);

	void run();
private:
	void accept();

	boost::asio::thread_pool m_workers_pool;
	unsigned int m_thread_count;
	net::io_context m_io_context;
	tcp::acceptor m_acceptor;
	std::shared_ptr<Router> m_router;
};