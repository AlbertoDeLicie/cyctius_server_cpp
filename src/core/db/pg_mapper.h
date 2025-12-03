#pragma once
#include <pqxx/pqxx>

namespace db {
	template<typename T>
	struct PgRowMapper {
		static T map(const pqxx::row& row);
	};
}