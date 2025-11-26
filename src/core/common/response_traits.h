#pragma once

#include <string>
#include <boost/beast/core/error.hpp>
#include <nlohmann/json.hpp>

template<typename ContentType>
struct ResponseTraits {
	using BodyType = std::string;

	static std::string serialize(const BodyType& r, boost::beast::error_code& code) {
		return r;
	}

};

template<>
struct ResponseTraits<nlohmann::json> {
	using BodyType = nlohmann::json;

	static std::string serialize(const BodyType& r, boost::beast::error_code& code) {
		try {
			return nlohmann::to_string(r);
		}
		catch (...) {
			code = boost::beast::http::error::unexpected_body;
		}
	}
};

template<>
struct ResponseTraits<int> {
	using BodyType = int;

	static std::string serialize(const BodyType& r, boost::beast::error_code& code) {
		return std::to_string(r);
	}
};

struct EmptyResponseBody {};

template<>
struct ResponseTraits<EmptyResponseBody> {
	using BodyType = EmptyResponseBody;

	static std::string serialize(const BodyType&, boost::beast::error_code& code) {
		return "";
	}
};