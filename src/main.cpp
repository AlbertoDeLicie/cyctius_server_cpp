#pragma once
#include <iostream>
#include "core/async/http_server.h"
#include "hello_route.h"
#include "core/common/logger.h"
#include "stl_traits.h"

int main() {

	/*
	init_async_logging(spdlog::level::level_enum::debug);

	auto router = std::make_shared<Router>();
	auto hello_route = std::make_shared<HelloRoute>();

	router->add_route<JsonContent>(
		boost::beast::http::verb::get,
		"/hello",
		[hello_route](std::shared_ptr<HttpRequest<JsonContent>> req) {
			return hello_route->hello_json(std::move(req));
		}
	);

	router->add_route<JsonContent>(
		boost::beast::http::verb::get,
		"/hello_throw",
		[hello_route](std::shared_ptr<HttpRequest<JsonContent>> req) {
			return hello_route->hello_throw(std::move(req));
		}
	);

	HttpServer server(8080, std::thread::hardware_concurrency());
	server.set_router(router);
	server.run();
	*/

	return 0;
}