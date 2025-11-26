#include "http_server.h"
#include "http_session.h"
#include <spdlog/spdlog.h>
#include "../common/socket_utils.h"

HttpServer::HttpServer(unsigned short port, unsigned int thread_count) :
	m_io_context(thread_count),
	m_thread_count(thread_count),
	m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port)),
	m_workers_pool(thread_count)
{
}

HttpServer::~HttpServer()
{
	m_workers_pool.join();
}

void HttpServer::set_router(std::shared_ptr<Router> router)
{
	m_router = router;
}

void HttpServer::run() {
	if (!m_router)
		throw std::invalid_argument("router is null!");

	accept();

	std::vector<std::thread> threads;
	threads.reserve(m_thread_count);

	for (unsigned int i = 0; i < m_thread_count; i++) {
		threads.emplace_back([this] {
			m_io_context.run();
			});
	}

	spdlog::info("server started and listening at: {}", m_acceptor.local_endpoint().port());

	for (auto& t : threads)
		t.join();
}

void HttpServer::accept() {
	m_acceptor.async_accept(
		[this](beast::error_code ec, tcp::socket socket) {

			auto ip_port = get_ip_port(socket);

			if (!ec) {
				spdlog::info("connection accepted: from {}:{}", ip_port.first, ip_port.second);
				std::make_shared<HttpSession>(std::move(socket), m_workers_pool, m_router)->start();
			}
			else {
				spdlog::error("connection failed for {}:{} because: ", ip_port.first, ip_port.second, ec.message());
			}

			accept();
		}
	);
}
