#include <boost/beast.hpp>
#include <boost/url.hpp>
#include <algorithm>
#include "router.h"
#include <spdlog/spdlog.h>

std::shared_ptr<RawResponse> Router::handle_request(const RawRequest& request) const noexcept
{
	try {
		boost::urls::url_view url(request.target());

		auto path = std::string(url.path());

		bool ok = false;
		auto& route = get_route_handler(request.method(), std::move(path), ok);

		if (ok) {
			auto response = route.handler(request);

			if (response) {
				return response;
			}
			else {
				return HttpResponse<std::string>::internal_server_error("500 Internal Server Error");
			}
		}
		else {
			return HttpResponse<std::string>::bad_request("404 Bad Request");
		}
	}
	catch (const std::exception& e) {
		return HttpResponse<std::string>::internal_server_error(e.what());
	}
	catch (...) {
		return HttpResponse<std::string>::internal_server_error("500 Internal Server Error");
	}
}

const Route& Router::get_route_handler(boost::beast::http::verb verb, std::string path, bool& ok) const noexcept
{
	auto iter = std::find_if(m_routes.cbegin(), m_routes.cend(), [&](const Route& r1) {
		return r1.path == path && r1.method == verb;
		});

	if (iter == m_routes.end()) {
		ok = false;
		return Route();
	}

	ok = true;
	return *iter;
}

