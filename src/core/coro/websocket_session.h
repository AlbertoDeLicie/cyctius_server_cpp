#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <iostream>
#include "http_router_coro.h"
#include "websocket_router.h"

namespace beast = boost::beast;

// Echoes back all received WebSocket messages
class WebsocketSession : public std::enable_shared_from_this<WebsocketSession>
{
public:
	// Take ownership of the socket
	explicit WebsocketSession(boost::asio::io_context& io, boost::asio::ip::tcp::socket socket, std::shared_ptr<WebsocketRouter> router);
	// Конструктор для использования в DetectSession
	// parser не муваем потому-что у него удален конструктор перемещения. к тому-же значение из него уже прочитано
	explicit WebsocketSession(
		boost::asio::io_context& io,
		boost::asio::ip::tcp::socket socket,
		std::shared_ptr<WebsocketRouter> router,
		beast::flat_buffer buffer
	) noexcept;

	~WebsocketSession();

	WebsocketSession(const WebsocketSession&) = delete;
	WebsocketSession& operator=(const WebsocketSession&) = delete;

	void start(beast::http::request<beast::http::dynamic_body> req);
	void close(boost::beast::websocket::close_reason reason);

private:
	boost::asio::awaitable<void> async_wait(beast::http::request<beast::http::dynamic_body> req);
	boost::asio::awaitable<void> async_read(std::string path);
	boost::asio::awaitable<void> async_close(boost::beast::websocket::close_reason reason);
	boost::asio::awaitable<void> async_write(std::shared_ptr<WebsocketRawMessage> message);

	boost::asio::io_context& m_io_context;

	beast::flat_buffer m_read_buffer;
	beast::flat_buffer m_write_buffer;

	boost::beast::websocket::stream<beast::tcp_stream> m_websocket_stream;
	std::shared_ptr<WebsocketRouter> m_router;
};