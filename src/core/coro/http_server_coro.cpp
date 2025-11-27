#include "http_server_coro.h"

#include <spdlog/spdlog.h>
#include "../common/socket_utils.h"
#include "detect_session.h"

using namespace boost::asio::ip;

HttpServerCoro::HttpServerCoro(unsigned short port, unsigned int thread_count) :
	m_io_context(thread_count),
	m_thread_count(thread_count),
	m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port)),
	m_router(std::make_shared<RouterCoro>(m_io_context))
{
}

HttpServerCoro::~HttpServerCoro()
{
	
}

void HttpServerCoro::run() {
	if (!m_router)
		throw std::invalid_argument("router is null!");

	boost::asio::co_spawn(m_io_context, accept(), boost::asio::detached);

	std::vector<std::thread> threads;
	threads.reserve(m_thread_count);

	for (unsigned int i = 0; i < m_thread_count; i++) {
		threads.emplace_back([this] { m_io_context.run(); });
	}

	spdlog::info("server started and listening at: {}", m_acceptor.local_endpoint().port());

	for (auto& t : threads)
		t.join();
}

std::shared_ptr<RouterCoro> HttpServerCoro::get_router()
{
	return m_router;
}

boost::asio::awaitable<void> HttpServerCoro::accept() {
	try {
		for (;;) {
			tcp::socket socket = co_await m_acceptor.async_accept(boost::asio::use_awaitable);

			auto ip_port = get_ip_port(socket);
			spdlog::info("connection accepted: {}:{}", ip_port.first, ip_port.second);

			auto session = std::make_shared<DetectSession>(m_io_context, std::move(socket), m_router);
			session->start();

			//m_sessions.emplace_back(session);

			//co_await clean_expired_sessions();
		}
	}
	catch (const std::exception& e) {
		spdlog::error("accept loop error: {}", e.what());
	}
}

//boost::asio::awaitable<void> HttpServerCoro::clean_expired_sessions()
//{
//	m_sessions.erase(std::remove_if(m_sessions.begin(), m_sessions.end(),
//		[](const std::weak_ptr<HttpSessionCoro>& s) { return s.expired(); }),
//		m_sessions.end());
//
//	co_return;
//}
//
//void HttpServerCoro::terminate_session(size_t i) const
//{
//	if (i >= m_sessions.size())
//		return;
//
//	auto session_weak = m_sessions[i];
//
//	if (session_weak.expired())
//		return;
//
//	auto session = session_weak.lock();
//
//	session->close();
//}
