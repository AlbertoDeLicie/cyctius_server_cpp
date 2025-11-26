#pragma once
#include <string>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "http_response.h"

using RawRequest = boost::beast::http::request<boost::beast::http::dynamic_body>;
using ResponsePtr = std::shared_ptr<RawResponse>;
using Handler = std::function<ResponsePtr(const RawRequest&)>;
using HandlerCoro = std::function<boost::asio::awaitable<ResponsePtr>(const RawRequest&)>;

struct Route {
	boost::beast::http::verb method;
	std::string path;
	Handler handler;

	bool operator==(const Route& other) const {
		return method == other.method && path == other.path;
	}
};

struct RouteCoro {
	boost::beast::http::verb method;
	std::string path;
	HandlerCoro handler;

	bool operator==(const RouteCoro& other) const {
		return method == other.method && path == other.path;
	}
};

