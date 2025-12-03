#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "http_router_coro.h"

namespace beast = boost::beast;

class HttpSessionCoro : public std::enable_shared_from_this<HttpSessionCoro> {

public:
	explicit HttpSessionCoro(boost::asio::io_context& io, boost::asio::ip::tcp::socket socket, std::shared_ptr<HttpRouterCoro> router) noexcept;

	// Конструктор для использования в DetectSession
	// parser не муваем потому-что у него удален конструктор перемещения. к тому-же значение из него уже прочитано
	explicit HttpSessionCoro(
		boost::asio::io_context& io, 
		boost::asio::ip::tcp::socket socket, 
		std::shared_ptr<HttpRouterCoro> router,
		beast::flat_buffer buffer
	) noexcept;

	~HttpSessionCoro();

	HttpSessionCoro(const HttpSessionCoro&) = delete;
	HttpSessionCoro& operator=(const HttpSessionCoro&) = delete;

	void start();
	void close();

	void handle_request(beast::http::request<beast::http::dynamic_body> req);

private:
	boost::asio::awaitable<void> async_read();
	boost::asio::awaitable<void> async_close();
	boost::asio::awaitable<void> async_handle_request(beast::http::request<beast::http::dynamic_body> req);

	boost::asio::io_context& m_io_context;

	boost::asio::ip::tcp::socket m_socket;
	std::shared_ptr<HttpRouterCoro> m_router;

	beast::flat_buffer m_buffer;
	std::optional<beast::http::request_parser<beast::http::dynamic_body>> m_parser;
	beast::http::response<beast::http::string_body> m_response;
};
