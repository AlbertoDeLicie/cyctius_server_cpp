#pragma once
#include <nlohmann/json.hpp>

template<typename T>
struct JsonMapper {
	static nlohmann::json map(const T& t) {
		return nlohmann::json();
	}

	static T map(const nlohmann::json& j) {
		return T();
	}
};
