#pragma once
#include <string>
#include <unordered_map>
#include <memory>

#include "../common/http_response.h"
#include "../common/http_request.h"
#include "../common/route.h"

class Router {
public:
	std::shared_ptr<RawResponse> handle_request(const RawRequest& request) const noexcept;

	template<typename BodyType, typename HandlerType>
	void add_route(boost::beast::http::verb verb, std::string path, HandlerType handler) {
		m_routes.emplace_back(
			verb,
			std::move(path),
			[handler](const RawRequest& req) {
				auto typed = from_request<BodyType>(req);
				return handler(typed);
			}
		);
	}

private:
	const Route& get_route_handler(boost::beast::http::verb verb, std::string path, bool& ok) const noexcept;
	std::vector<Route> m_routes;
};
