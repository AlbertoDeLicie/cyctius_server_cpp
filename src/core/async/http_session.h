#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "router.h"

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
	HttpSession(net::ip::tcp::socket socket, boost::asio::thread_pool& pool, std::shared_ptr<Router> router);
	HttpSession(const HttpSession&) = delete;
	HttpSession& operator=(const HttpSession&) = delete;

	void start();

private:
	void read();
	void close();
	void handle_request(beast::http::request<beast::http::dynamic_body> req);

	tcp::socket m_socket;
	std::shared_ptr<Router> m_router;
	beast::flat_buffer m_buffer;
	std::optional<beast::http::request_parser<beast::http::dynamic_body>> m_parser;
	beast::http::response<beast::http::string_body> m_response;

	boost::asio::strand<boost::asio::any_io_executor> m_strand;
	boost::asio::thread_pool& m_workers_pool;
};