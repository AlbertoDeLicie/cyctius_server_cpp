#pragma once
#include <string>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "http_response.h"

using RawRequest = boost::beast::http::request<boost::beast::http::dynamic_body>;
using ResponsePtr = std::shared_ptr<RawResponse>;

using Handler = std::function<ResponsePtr(const RawRequest&)>;
using HandlerCoro = std::function<boost::asio::awaitable<ResponsePtr>(const RawRequest&)>;

struct HttpRoute {
	boost::beast::http::verb method;
	std::string path;
	Handler handler;

	bool operator==(const HttpRoute& other) const {
		return method == other.method && path == other.path;
	}
};

struct HttpRouteCoro {
	boost::beast::http::verb method;
	std::string path;
	HandlerCoro handler;

	bool operator==(const HttpRouteCoro& other) const {
		return method == other.method && path == other.path;
	}
};

