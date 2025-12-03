#pragma once

#include "core/db/pg_mapper.h"
#include "core/transform/mapper.h"
#include "core/common/response_traits.h"

struct User {
	int id;
	std::string name;
	int age;
};

template<>
struct db::PgRowMapper<User> {
	static User map(const pqxx::row& row) {
		return {
			row["id"].as<int>(),
			row["name"].as<std::string>(),
			row["age"].as<int>()
		};
	}
};

template<>
struct JsonMapper<User> {
	static nlohmann::json map(const User& user) {
		nlohmann::json result;

		result["id"] = user.id;
		result["name"] = user.name;
		result["age"] = user.age;

		return result;
	}
};

template<>
struct JsonMapper<std::vector<User>> {
	static nlohmann::json map(const std::vector<User>& users) {
		nlohmann::json result;
		nlohmann::json array = nlohmann::json::array();

		for (const auto& user : users) {
			auto json_user = JsonMapper<User>::map(user);
			array.push_back(std::move(json_user));
		}

		result["users"] = std::move(array);
		return result;
	}
};
