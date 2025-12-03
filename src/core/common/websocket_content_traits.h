#pragma once
#include <vector>
#include "../transform/mapper.h"

using WebsocketRawMessage = std::vector<uint8_t>;

// by default try to read as json
template<typename Type>
struct WebsocketContentTraits {
	using MessageBodyType = WebsocketRawMessage;

	static MessageBodyType serialize(const Type& data, boost::beast::error_code& code) {
		try {
			auto j = JsonMapper<Type>::map(data).dump();
			return std::vector<uint8_t>(j.begin(), j.end());
		}
		catch (std::exception& ex) {
			code = boost::asio::error::invalid_argument;
			return MessageBodyType();
		}
	}

	static Type deserialize(const MessageBodyType& data, boost::beast::error_code& code) {
		try {
			std::string str(data.begin(), data.end());
			nlohmann::json json = nlohmann::json::parse(str);
			auto type_object = JsonMapper<Type>::map(json);
			return type_object;
		}
		catch (std::exception& ex) {
			code = boost::asio::error::invalid_argument;
			return Type();
		}
	}
};



