#include "websocket_router.h"
#include <spdlog/spdlog.h>

WebsocketRouter::WebsocketRouter(boost::asio::io_context& io) noexcept :
	m_io_context(io)
{

}

boost::asio::awaitable<std::shared_ptr<WebsocketRawMessage>> 
WebsocketRouter::handle_request(
	std::string path,
	const WebsocketRawMessage& request) const noexcept
{
	try {
		bool ok = false;
		auto& route = get_route_handler(path, ok);

		if (ok) {
			co_return co_await route.handler(request);
		}
	}
	catch (const std::exception& e) {
		spdlog::error("Error handle websocket request: {}", e.what());
		co_return nullptr;
	}
	catch (...) {
		spdlog::error("Error handle websocket request: {}", "i don't know what happend :(");
		co_return nullptr;
	}

	co_return nullptr;
}

const WebsocketRoute& WebsocketRouter::get_route_handler(std::string path, bool& ok) const noexcept
{
	auto iter = m_routes.find(path);

	if (iter == m_routes.end()) {
		ok = false;
		return {};
	}

	ok = true;
	return iter->second;
}
