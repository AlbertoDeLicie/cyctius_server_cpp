#include "http_session_coro.h"
#include <spdlog/spdlog.h>
#include "../common/socket_utils.h"

using namespace boost::asio;

HttpSessionCoro::HttpSessionCoro(boost::asio::io_context& io, ip::tcp::socket socket, std::shared_ptr<RouterCoro> router) :
	m_io_context(io),
	m_socket(std::move(socket)),
	m_router(std::move(router))
{
}

void HttpSessionCoro::start() {
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_io_context,
		[self]() -> boost::asio::awaitable<void> {
			co_await self->read();
		},
		boost::asio::detached
	);
}

boost::asio::awaitable<void> HttpSessionCoro::read() {
	try {
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
		auto self = shared_from_this();
		boost::asio::co_spawn(
			m_io_context,
			handle_request(std::move(req)),
			boost::asio::detached
		);
	}
	catch (std::exception& ex) {
		co_return;
	}
}

boost::asio::awaitable<void> HttpSessionCoro::close() {
	try {
		if (m_socket.is_open()) {
			auto ip_port = get_ip_port(m_socket);
			boost::system::error_code ec;
			m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			m_socket.close(ec);

			if (ec) {
				spdlog::error("Error closing socket: {}", ec.message());
			}
			else {
				spdlog::info("connection closed: {}:{}", ip_port.first, ip_port.second);
			}
		}
	}
	catch (const std::exception& e) {
		spdlog::error("Exception in close(): {}", e.what());
	}
	co_return;
}

boost::asio::awaitable<void> HttpSessionCoro::handle_request(beast::http::request<beast::http::dynamic_body> req) {
	try {
		auto self = shared_from_this();
		auto response = co_await m_router->handle_request(std::move(req));

		bool keep_alive = req.keep_alive();

		co_await beast::http::async_write(m_socket, *response, boost::asio::use_awaitable);

		if (keep_alive && m_socket.is_open()) {
			boost::asio::co_spawn(
				m_io_context,
				[self]() -> boost::asio::awaitable<void> {
					co_await self->read();
				},
				boost::asio::detached
			);
		}
		else {
			co_await close();
		}
	}
	catch (std::exception& e) {
		spdlog::error("Exception in handle_request_async: {}", e.what());
	}
}

