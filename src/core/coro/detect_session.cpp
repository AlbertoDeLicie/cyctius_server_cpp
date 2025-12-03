#include "detect_session.h"
#include "http_session_coro.h"
#include "websocket_session.h"
#include <spdlog/spdlog.h>

DetectSession::DetectSession(
	boost::asio::io_context& io, 
	boost::asio::ip::tcp::socket socket, 
	std::shared_ptr<HttpRouterCoro> router,
	std::shared_ptr<WebsocketRouter> ws_router
) noexcept :
	m_io_context(io),
	m_socket(std::move(socket)),
	m_router(std::move(router)),
	m_ws_router(std::move(ws_router))
{
}

DetectSession::~DetectSession()
{
}

boost::asio::awaitable<void> DetectSession::async_read() 
{
	try {
		auto self = shared_from_this();

		m_parser.emplace();
		m_parser->body_limit(1024 * 1000 * 1000);

		co_await beast::http::async_read(
			m_socket,
			m_buffer,
			*m_parser,
			boost::asio::use_awaitable
		);

		if (!m_socket.is_open())
			co_return;

		auto req = m_parser->release();

		// запускаем обработку запроса
		co_await async_start_specific_session(std::move(req));
	}
	catch (std::exception& ex) {
		co_return;
	}
}

boost::asio::awaitable<void> DetectSession::async_start_specific_session(beast::http::request<beast::http::dynamic_body> req)
{
	auto upgrade_header = req[boost::beast::http::field::upgrade];

	// судя по всему обычный http запрос
	if (upgrade_header.empty()) {
		auto http_session = std::make_shared<HttpSessionCoro>(m_io_context, std::move(m_socket), m_router, std::move(m_buffer));
		http_session->handle_request(std::move(req));
		co_return;
	}
	else if (upgrade_header == "websocket") {
		auto websocket_session = std::make_shared<WebsocketSession>(m_io_context, std::move(m_socket), m_ws_router, std::move(m_buffer));
		websocket_session->start(std::move(req));
		co_return;
	}

	co_return;
}

void DetectSession::start() {
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_io_context,
		[self]() -> boost::asio::awaitable<void> {
			co_await self->async_read();
		},
		boost::asio::detached
	);
}