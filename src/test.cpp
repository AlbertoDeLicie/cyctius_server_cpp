#include <iostream>
#include <boost/asio.hpp>
#include "core/db/pg_data_base.h"
#include <string>
#include <random>
#include <numeric>

const std::string DB_CONN_STR = "dbname=cyctius user=postgres password=password host=LOCALHOST port=5433";

class UserRepository {
public:
	UserRepository(std::shared_ptr<db::PgDataBase<db::RoundRobinPolicy>> db_conn) : db_conn_(db_conn) {}

	boost::asio::awaitable<std::optional<std::vector<User>>> find_all_users(std::shared_ptr<boost::system::error_code> ec) {
		std::string query = "SELECT id, name, age FROM users";
		auto users = co_await db_conn_->async_execute<User>(query, ec);

		//std::cout << users.value().front().name << std::endl;

		if (ec) {
			co_return std::nullopt;
		}
		co_return users;
	}

private:
	std::shared_ptr<db::PgDataBase<db::RoundRobinPolicy>> db_conn_;
};

void request_handler(UserRepository& user_service, boost::asio::io_context& ioc) {

	auto ec = std::make_shared<boost::system::error_code>();
	boost::asio::co_spawn(ioc, user_service.find_all_users(ec), boost::asio::detached);
}

int main() {

	boost::asio::io_context ioc(10);

	db::ConnectionInfo con_info{
		"LOCALHOST",
		"cyctius",
		"postgres",
		"password",
		5433,
		"cyctius_session",
		20
	};

	auto database = std::make_shared<db::PgDataBase<db::RoundRobinPolicy>>(ioc, con_info);
	UserRepository user_repo(database);

	database->connect_all();

	auto work = boost::asio::make_work_guard(ioc);

	std::vector<std::thread> threads;
	for (int i = 0; i < 10; i++) {
		threads.push_back(std::thread([&]() {
			ioc.run();
			}));
	}

	std::random_device rd; // Инициализация генератора
	std::mt19937 mt(rd()); // Mersenne Twister engine
	std::uniform_int_distribution<int> dist(8, 20); // Распределение от 8 до 20 

	std::vector<std::thread> db_threads;
	const int num_threads = 100;

	bool stop = false;

	for (int i = 0; i < num_threads; ++i) {
		threads.emplace_back(
			[&]() {
				while (!stop) {
					request_handler(user_repo, ioc);
					// Генерация случайной задержки для текущего потока в миллисекундах
					std::chrono::milliseconds delay(dist(mt));
					std::this_thread::sleep_for(delay);
				}
			}
		);
	}

	std::string command;
	while (true) {
		std::cout << "> ";        // приглашение к вводу
		std::getline(std::cin, command);
	
		if (command == "rpt") {
			request_handler(user_repo, ioc);
		}
		else if (command == "cls") {
			//database->terminate();

			stop = true;
		}
		else if (command == "str") {
			database->connect_all();
		}
		else {
			std::cout << "Unknown command\n";
		}
	}

	work.reset();
	ioc.stop();

	for (auto& t : threads) {
		if (t.joinable()) {
			t.join();
		}
	}

	for (auto& t : db_threads) {
		if (t.joinable()) {
			t.join();
		}
	}

	return 0;
}