#include "http_session.h"
#include <iostream>
#include "../common/http_request.h"
#include "../common/http_response.h"
#include "../common/socket_utils.h"

#include <spdlog/spdlog.h>

HttpSession::HttpSession(net::ip::tcp::socket socket, boost::asio::thread_pool& pool, std::shared_ptr<Router> router) :
	m_socket(std::move(socket)),
	m_router(std::move(router)),
	m_strand(m_socket.get_executor()),
	m_workers_pool(pool)
{

}

void HttpSession::read()
{
	m_parser.emplace();
	m_parser->body_limit(1024 * 1000 * 1000);

	auto self = shared_from_this();
	beast::http::async_read(
		m_socket,
		m_buffer,
		*m_parser,
		boost::asio::bind_executor(
			m_strand,
			[self](beast::error_code ec, std::size_t) {
				if (!ec && self->m_socket.is_open()) {
					self->handle_request(std::move(self->m_parser->release()));
				} 
				else {
					auto ip_port = get_ip_port(self->m_socket);
					spdlog::error("can't read data from cleint: {}:{} because {}", ip_port.first, ip_port.second, ec.message());
					self->close();
				}
			})
	);
}

void HttpSession::start()
{
	read();
}

void HttpSession::close()
{
	beast::error_code ec;
	m_socket.shutdown(tcp::socket::shutdown_send, ec);
	m_socket.close(ec);
}

void HttpSession::handle_request(beast::http::request<beast::http::dynamic_body> req)
{
	auto self = shared_from_this();
	boost::asio::post(
		m_workers_pool,
		[self, req = std::move(req)]() mutable {
			auto response = self->m_router->handle_request(req);

			auto keep_alive = req.keep_alive();
			beast::http::async_write(
				self->m_socket,
				*response,
				boost::asio::bind_executor(
					self->m_strand,
					[self, response, keep_alive](beast::error_code ec, std::size_t size) {
						if (ec) {
							auto message = ec.message();
							spdlog::info("can't write to socket beacuse: {}", message);

							self->m_socket.shutdown(tcp::socket::shutdown_send, ec);
							self->m_socket.close();
							return;
						}

						if (keep_alive && self->m_socket.is_open()) {
							self->read();
						}
						else {
							auto ip_port = get_ip_port(self->m_socket);
							spdlog::info("connection was closed for: {}:{}", ip_port.first, ip_port.second);

							self->m_socket.shutdown(tcp::socket::shutdown_send, ec);
							self->m_socket.close();
						}
					}
				));
		}
	);
}
