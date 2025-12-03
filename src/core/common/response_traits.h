#pragma once

#include <string>
#include <boost/beast/core/error.hpp>
#include <nlohmann/json.hpp>
#include "../transform/mapper.h"

template<typename ContentType>
struct HttpResponseTraits {
	using BodyType = ContentType;

	static std::string serialize(const BodyType& r, boost::beast::error_code& code) {
		try {
			return JsonMapper<ContentType>::map(r).dump();
		}
		catch (std::exception& ex) {
			code = boost::asio::error::invalid_argument;
			return "";
		}
	}

};

template<>
struct HttpResponseTraits<nlohmann::json> {
	using BodyType = nlohmann::json;

	static std::string serialize(const BodyType& r, boost::beast::error_code& code) {
		try {
			return nlohmann::to_string(r);
		}
		catch (...) {
			code = boost::asio::error::invalid_argument;
			return {};
		}
	}
};

template<>
struct HttpResponseTraits<int> {
	using BodyType = int;

	static std::string serialize(const BodyType& r, boost::beast::error_code& code) {
		return std::to_string(r);
	}
};

struct EmptyResponseBody {};

template<>
struct HttpResponseTraits<EmptyResponseBody> {
	using BodyType = EmptyResponseBody;

	static std::string serialize(const BodyType&, boost::beast::error_code& code) {
		return "";
	}
};