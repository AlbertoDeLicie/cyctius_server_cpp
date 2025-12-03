#pragma once
#include <string>
#include <vector>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "websocket_message.h"

struct WebsocketRoute {
	using Handler = std::function<boost::asio::awaitable<std::shared_ptr<WebsocketRawMessage>>(const WebsocketRawMessage& message)>;
	Handler handler;
};