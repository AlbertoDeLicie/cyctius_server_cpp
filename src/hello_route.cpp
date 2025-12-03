#include "hello_route.h"
#include "core/transform/mapper.h"
#include "user.h"


const db::ConnectionInfo db_connection_info {
		"LOCALHOST",
		"cyctius",
		"postgres",
		"password",
		5433,
		"cyctius_session",
		2
};

HelloRoute::HelloRoute(boost::asio::io_context& io_context) :
	m_io_context(io_context),
	m_db(std::make_shared<db::PgDataBase<db::RoundRobinPolicy>>(io_context, db_connection_info))
{
	m_db->connect_lazy();
}

boost::asio::awaitable<std::shared_ptr<RawResponse>> HelloRoute::hello_json(std::shared_ptr<HttpRequest<JsonContent>> req)
{
	if (!req) {
		co_return HttpResponse<std::string>::bad_request("Bad Request!");
	}

	std::string query = "SELECT id, name, age FROM users";

	auto ec = std::make_shared<boost::system::error_code>();
	auto users = co_await m_db->async_execute<User>(query, ec);

	if (!users.has_value())
		co_return HttpResponse<User>::bad_request("Users not found!");

	HttpResponse<std::vector<User>> response(11, boost::beast::http::status::ok);
	response.set_body(std::move(users.value()));

	co_return response.build();
}

boost::asio::awaitable<std::shared_ptr<WebsocketRawMessage>> HelloRoute::ws_hello_json(std::shared_ptr<Message> msg)
{
	if (!msg)
		co_return nullptr;

	Message rep_msg{ "Echo: " + msg->message };

	co_return to_message<Message>(rep_msg);
}

