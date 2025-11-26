#include "router_coro.h"

RouterCoro::RouterCoro(boost::asio::io_context& io) noexcept :
	m_io_context(io)
{

}

boost::asio::awaitable<std::shared_ptr<RawResponse>> RouterCoro::handle_request(const RawRequest& request) const noexcept {
	try {
		boost::urls::url_view url(request.target());

		auto path = std::string(url.path());

		bool ok = false;
		auto& route = get_route_handler(request.method(), std::move(path), ok);

		if (ok) {
			auto response = co_await route.handler(request);

			if (response) {
				co_return response;
			}
			else {
				co_return HttpResponse<std::string>::internal_server_error("500 Internal Server Error");
			}
		}
		else {
			co_return HttpResponse<std::string>::bad_request("404 Bad Request");
		}
	}
	catch (const std::exception& e) {
		co_return HttpResponse<std::string>::internal_server_error(e.what());
	}
	catch (...) {
		co_return HttpResponse<std::string>::internal_server_error("500 Internal Server Error");
	}
}

const RouteCoro& RouterCoro::get_route_handler(boost::beast::http::verb verb, std::string path, bool& ok) const noexcept {
	auto iter = std::find_if(m_routes.cbegin(), m_routes.cend(), [&](const RouteCoro& r1) {
		return r1.path == path && r1.method == verb;
		});

	if (iter == m_routes.end()) {
		ok = false;
		return RouteCoro();
	}

	ok = true;
	return *iter;
}