#pragma once
#include <boost/asio.hpp>
#include "core/common/http_response.h"
#include "core/common/http_request.h"

class HelloRoute {
public:
    boost::asio::awaitable<std::shared_ptr<RawResponse>> hello_json(std::shared_ptr<HttpRequest<JsonContent>> req);
};
