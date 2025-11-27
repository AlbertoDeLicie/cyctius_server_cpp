#include "http_session_coro.h"
#include <spdlog/spdlog.h>
#include "../common/socket_utils.h"

using namespace boost::asio;

HttpSessionCoro::HttpSessionCoro(boost::asio::io_context& io, ip::tcp::socket socket, std::shared_ptr<RouterCoro> router) noexcept :
	m_io_context(io),
	m_socket(std::move(socket)),
	m_router(std::move(router))
{
}

HttpSessionCoro::HttpSessionCoro(
	boost::asio::io_context& io, 
	boost::asio::ip::tcp::socket socket, 
	std::shared_ptr<RouterCoro> router, 
	beast::flat_buffer buffer
) noexcept :
	m_io_context(io),
	m_socket(std::move(socket)),
	m_router(std::move(router)),
	m_buffer(std::move(buffer))
{
}

HttpSessionCoro::~HttpSessionCoro()
{
	spdlog::debug("Http session destructed");
}

void HttpSessionCoro::start() {
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_io_context,
		[self]() -> boost::asio::awaitable<void> {
			co_await self->async_read();
		},
		boost::asio::detached
	);
}

void HttpSessionCoro::handle_request(beast::http::request<beast::http::dynamic_body> req) {
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_io_context,
		[self, req_ = std::move(req)]() -> boost::asio::awaitable<void> {
			co_await self->async_handle_request(req_);
		},
		boost::asio::detached
	);
}

void HttpSessionCoro::close() {
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_io_context,
		[self]() -> boost::asio::awaitable<void> {
			co_await self->async_close();
		},
		boost::asio::detached
	);
}

boost::asio::awaitable<void> HttpSessionCoro::async_read() {
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
		co_await async_handle_request(std::move(req));
	}
	catch (std::exception& ex) {
		co_return;
	}
}

boost::asio::awaitable<void> HttpSessionCoro::async_close() {
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

boost::asio::awaitable<void> HttpSessionCoro::async_handle_request(beast::http::request<beast::http::dynamic_body> req) {
	try {
		auto response = co_await m_router->handle_request(std::move(req));

		bool keep_alive = req.keep_alive();

		co_await beast::http::async_write(m_socket, *response, boost::asio::use_awaitable);

		if (keep_alive && m_socket.is_open()) {
			co_await async_read();
		}
		else {
			co_await async_close();
		}
	}
	catch (std::exception& e) {
		spdlog::error("Exception in handle_request_async: {}", e.what());
	}
}

