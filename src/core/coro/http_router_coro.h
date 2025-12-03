#pragma once
#include <boost/asio.hpp>
#include <string>
#include <unordered_map>
#include <memory>

#include "../common/http_response.h"
#include "../common/http_request.h"
#include "../common/route.h"

class HttpRouterCoro {
public:
	HttpRouterCoro(boost::asio::io_context& io) noexcept;
	HttpRouterCoro(const HttpRouterCoro&) = delete;
	HttpRouterCoro& operator=(const HttpRouterCoro&) = delete;

	boost::asio::awaitable<std::shared_ptr<RawResponse>> handle_request(const RawRequest& request) const noexcept;

	template<typename BodyType, typename HandlerType>
	void add_route(boost::beast::http::verb verb, std::string path, HandlerType handler) {
		m_routes.emplace_back(
			verb,
			std::move(path),
			[handler](const RawRequest& req) -> boost::asio::awaitable<std::shared_ptr<RawResponse>> {
				auto typed = from_request<BodyType>(req);
				co_return co_await handler(typed);
			}
		);
	}

private:
	const HttpRouteCoro& get_route_handler(boost::beast::http::verb verb, std::string path, bool& ok) const noexcept;
	std::vector<HttpRouteCoro> m_routes;

	boost::asio::io_context& m_io_context;
};
