#include "hello_route.h"

boost::asio::awaitable<std::shared_ptr<RawResponse>> HelloRoute::hello_json(std::shared_ptr<HttpRequest<JsonContent>> req)
{
	if (!req) {
		co_return HttpResponse<std::string>::bad_request("Bad Request!");
	}

	nlohmann::json j = {
			{"name", "John Doe"},
			{"age", 30},
			{"is_student", false},
			{"skills", {"C++", "Python", "JavaScript"}},
			{
				"address", {
				{"street", "123 Main St"},
				{"city", "Anytown"}
			}
		}
	};
	
	HttpResponse<nlohmann::json> response(11, boost::beast::http::status::ok);
	response.set_body(j);

	co_return response.build();
}

