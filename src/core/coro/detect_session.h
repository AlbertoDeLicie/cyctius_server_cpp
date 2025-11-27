#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "router_coro.h"

namespace beast = boost::beast;

class DetectSession : public std::enable_shared_from_this<DetectSession> {
public:
	explicit DetectSession(boost::asio::io_context& io, boost::asio::ip::tcp::socket socket, std::shared_ptr<RouterCoro> router) noexcept;
	DetectSession(const DetectSession&) = delete;
	DetectSession& operator=(const DetectSession&) = delete;

	~DetectSession();

	void start();

private:
	boost::asio::awaitable<void> async_read();
	boost::asio::awaitable<void> async_start_specific_session(beast::http::request<beast::http::dynamic_body> req);

	std::shared_ptr<RouterCoro> m_router;

	boost::asio::io_context& m_io_context;
	boost::asio::ip::tcp::socket m_socket;
	beast::flat_buffer m_buffer;
	std::optional<beast::http::request_parser<beast::http::dynamic_body>> m_parser;
};