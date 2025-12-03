#pragma once
#include <boost/asio.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include "../common/websocket_route.h"
#include "../common/websocket_message.h"

class WebsocketRouter {
public:
	WebsocketRouter(boost::asio::io_context& io) noexcept;
	WebsocketRouter(const WebsocketRouter&) = delete;
	WebsocketRouter& operator=(const WebsocketRouter&) = delete;

	boost::asio::awaitable<std::shared_ptr<WebsocketRawMessage>> handle_request(
		std::string path,
		const WebsocketRawMessage& message) const noexcept;

	template<typename InputMessage, typename HandlerType>
	void add_route(std::string path, HandlerType handler) {
		m_routes.emplace(
			path,
			WebsocketRoute {
				[handler](const WebsocketRawMessage& message) -> boost::asio::awaitable<std::shared_ptr<WebsocketRawMessage>> {
					auto typed = from_message<InputMessage>(message);
					co_return co_await handler(typed);
				}
			}
		);
	}

private:
	const WebsocketRoute& get_route_handler(std::string path, bool& ok) const noexcept;
	std::unordered_map<std::string, WebsocketRoute> m_routes;

	boost::asio::io_context& m_io_context;

};