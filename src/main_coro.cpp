#pragma once
#include <iostream>
#include "core/coro/http_server_coro.h"
#include "core/common/logger.h"
#include "hello_route.h"

int main() {

	init_async_logging(spdlog::level::level_enum::debug);

	HttpServerCoro server(8080, std::thread::hardware_concurrency());

	auto router = server.get_router();
	auto hello_route = std::make_shared<HelloRoute>();

	router->add_route<JsonContent>(
		boost::beast::http::verb::get,
		"/hello",
		[hello_route](std::shared_ptr<HttpRequest<JsonContent>> req) {
			return hello_route->hello_json(std::move(req));
		}
	);

	server.run();

	return 0;
}