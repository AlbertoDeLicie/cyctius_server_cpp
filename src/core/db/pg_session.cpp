#include "pg_session.h"
#include "../common/socket_utils.h"

db::PgSession::PgSession(boost::asio::io_context& ioc, std::string conn_str) noexcept :
	m_io_context(ioc),
	m_connection_str(std::move(conn_str)),
	m_socket(ioc),
	m_strand(ioc.get_executor())
{

}

db::PgSession::~PgSession()
{
	//close();
}

void db::PgSession::start() {
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_strand,
		[self]() -> boost::asio::awaitable<void> {
			bool connected = co_await self->async_connect();
			if (connected) {
				self->assign_socket();
				co_await self->async_wait();
			} else {
				co_return;
			}
		},
		boost::asio::detached
	);
}

void db::PgSession::assign_socket()
{
	int file_descriptor = m_connection->sock();

	if (file_descriptor < 0) {
		throw std::runtime_error("database invalid connection");
	}

	m_socket.assign(boost::asio::ip::tcp::v4(), file_descriptor);
}

boost::asio::awaitable<void> db::PgSession::async_wait() {
	try {
		if (!is_open()) {
			spdlog::info("pg session is closed");
			co_return;
		}

		boost::system::error_code ec;
		co_await m_socket.async_wait(
			boost::asio::ip::tcp::socket::wait_read,
			boost::asio::redirect_error(boost::asio::use_awaitable, ec)
		);

		if (ec) {
			spdlog::info("pg session was closed");
			async_close();
			co_return;
		}

		co_await async_wait();
	}
	catch(std::exception& ex) {
		spdlog::error("pg session was bad close: {}", ex.what());
		async_close();
	}
}

boost::asio::awaitable<bool> db::PgSession::async_connect()
{
	try {
		if (m_connection && m_connection->is_open()) {
			spdlog::info("pg session already connected to: {}", m_connection_str);
			co_return false;
		}

		m_connection.emplace(m_connection_str);
		spdlog::info("pg session connected to: {}", m_connection_str);
		co_return true;
	}
	catch (...) {
		spdlog::error("pg session can't connect to: {}", m_connection_str);
		co_return false;
	}
}

bool db::PgSession::is_open() const
{
	if (!m_connection)
		return false;

	return m_connection->is_open() && m_socket.is_open();
}

void db::PgSession::close()
{
	auto self = shared_from_this();
	boost::asio::co_spawn(
		m_strand,
		[self]() -> boost::asio::awaitable<void> {
			co_await self->async_close();
			co_return;
		},
		boost::asio::detached
	);
}

boost::asio::awaitable<void> db::PgSession::async_close()
{
	try {
		auto self = shared_from_this();

		if (!is_open())
			co_return;

		auto ip_port = get_ip_port(m_socket);
		boost::system::error_code ec;
		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		m_socket.close(ec);

		if (ec) {
			spdlog::error("pg session error closing: {}", ec.message());
		}
		else {
			spdlog::info("pg session closed correctly: {}:{}", ip_port.first, ip_port.second);
		}

		// close позовется в деструкторе!
		m_connection.reset();

		co_return;
	}
	catch (std::exception& ex) {
		spdlog::error("pg session error closing: {}", ex.what());

	}
	catch (...) {
		spdlog::error("pg session bad closing");
	}
}