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
        User user;
        user.id = row["id"].as<int>();
        user.name = row["name"].as<std::string>();
        user.age = row["age"].as<int>();
        return user;
    }
};
