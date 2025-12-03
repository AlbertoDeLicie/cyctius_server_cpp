#include "websocket_session.h"
#include <spdlog/spdlog.h>

WebsocketSession::WebsocketSession(boost::asio::io_context& io, boost::asio::ip::tcp::socket socket, std::shared_ptr<WebsocketRouter> router) :
	m_io_context(io),
	m_websocket_stream(std::move(socket)),
	m_router(std::move(router))
{
}

WebsocketSession::WebsocketSession(
	boost::asio::io_context& io, 
	boost::asio::ip::tcp::socket socket, 
	std::shared_ptr<WebsocketRouter> router,
	beast::flat_buffer buffer
) noexcept :
	m_io_context(io),
	m_websocket_stream(std::move(socket)),
	m_router(std::move(router)),
	m_read_buffer(std::move(buffer))
{
}

WebsocketSession::~WebsocketSession()
{
	spdlog::debug("Websocket session destructed");
}


void WebsocketSession::start(beast::http::request<beast::http::dynamic_body> req) {
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_io_context,
		[self, req_ = std::move(req)]() -> boost::asio::awaitable<void> {
			co_await self->async_wait(req_);
		},
		boost::asio::detached
	);
}

void WebsocketSession::close(boost::beast::websocket::close_reason reason) {
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_io_context,
		[self, reason_ = std::move(reason)]() -> boost::asio::awaitable<void> {
			co_await self->async_close(reason_);
		},
		boost::asio::detached
	);
}

boost::asio::awaitable<void> WebsocketSession::async_wait(beast::http::request<beast::http::dynamic_body> req) {
	
	try {
		m_websocket_stream.set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::server));
		m_websocket_stream.set_option(beast::websocket::stream_base::decorator([](beast::websocket::response_type& res) {
				res.set(beast::http::field::server, "cyctius-websocket-server");
			}));

		co_await m_websocket_stream.async_accept(req);

		spdlog::info("websocket_session started");

		boost::urls::url_view url(req.target());
		auto path = std::string(url.path());

		co_await async_read(std::move(path));
	}
	catch (std::exception& ex) {
		spdlog::error("websocket_session error accept because: {}", ex.what());
		co_return;
	}
	catch (...) {
		spdlog::error("websocket_session error accept");
		co_return;
	}
}

boost::asio::awaitable<void> WebsocketSession::async_read(std::string path) {
	auto [ec, _] = co_await m_websocket_stream.async_read(m_read_buffer, boost::asio::as_tuple);

	if (ec == beast::websocket::error::closed) {
		spdlog::info("websocket_session closed because: {}", ec.message());
		co_return;
	}

	if (ec) {
		spdlog::info("websocket_session closed because: {}", ec.message());
		co_return;
	}
	
	WebsocketRawMessage raw;
	raw.resize(m_read_buffer.size());
	boost::asio::buffer_copy(boost::asio::buffer(raw), m_read_buffer.data());
	m_read_buffer.clear();

	auto response = co_await m_router->handle_request(path, raw);

	if (response) {
		co_await async_write(std::move(response));
	}

	co_await async_read(std::move(path));
}

boost::asio::awaitable<void> WebsocketSession::async_close(boost::beast::websocket::close_reason reason) {
	if (!m_websocket_stream.is_open())
		co_return;

	m_websocket_stream.close(reason);

	spdlog::info("websocket_session closed");

	co_return;
}

boost::asio::awaitable<void> WebsocketSession::async_write(std::shared_ptr<WebsocketRawMessage> message) {
	
	if (!message)
		co_return;
	
	m_write_buffer.clear();
	boost::asio::mutable_buffer writable_area = m_write_buffer.prepare(message->size());
	boost::asio::buffer_copy(writable_area, boost::asio::buffer(*message));
	m_write_buffer.commit(message->size());

	co_await m_websocket_stream.async_write(m_write_buffer.data());
}