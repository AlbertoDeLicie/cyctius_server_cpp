#pragma once
#include <pqxx/pqxx>

struct User {
	int id;
	std::string name;
	int age;
};

template<typename T>
struct PgRowMapper {
	static T map(const pqxx::row& row);
};

template<>
struct PgRowMapper<User> {
	static User map(const pqxx::row& row) {
		return {
			row["id"].as<int>(),
			row["name"].as<std::string>(),
			row["age"].as<int>()
		};
	}
};
