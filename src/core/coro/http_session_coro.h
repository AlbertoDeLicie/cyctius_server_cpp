#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "router_coro.h"

namespace beast = boost::beast;

class HttpSessionCoro : public std::enable_shared_from_this<HttpSessionCoro> {

public:
	HttpSessionCoro(boost::asio::io_context& io, boost::asio::ip::tcp::socket socket, std::shared_ptr<RouterCoro> router);
	HttpSessionCoro(const HttpSessionCoro&) = delete;
	HttpSessionCoro& operator=(const HttpSessionCoro&) = delete;

	void start();

private:
	boost::asio::awaitable<void> read();
	boost::asio::awaitable<void> close();
	boost::asio::awaitable<void> handle_request(beast::http::request<beast::http::dynamic_body> req);

	boost::asio::io_context& m_io_context;

	boost::asio::ip::tcp::socket m_socket;
	std::shared_ptr<RouterCoro> m_router;

	beast::flat_buffer m_buffer;
	std::optional<beast::http::request_parser<beast::http::dynamic_body>> m_parser;
	beast::http::response<beast::http::string_body> m_response;
};
