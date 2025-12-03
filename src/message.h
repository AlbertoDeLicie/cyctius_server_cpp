#pragma once

#include "core/transform/mapper.h"
#include "core/common/response_traits.h"

struct Message {
	std::string message;
};

template<>
struct JsonMapper<Message> {
	static nlohmann::json map(const Message& msg) {
		nlohmann::json result;

		result["message"] = msg.message;

		return result;
	}

	static Message map(const nlohmann::json& j) {
		Message msg;

		j.at("message").get_to(msg.message);

		return msg;
	}
};