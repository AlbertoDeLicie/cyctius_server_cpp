#pragma once
#include "pg_session.h"
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <mutex>

namespace db {
	struct ConnectionInfo {
		std::string host;
		std::string dbname;
		std::string user;
		std::string password;
		unsigned int port;
		std::string app_name;
		unsigned int max_sessions = 3;
	};

	struct RoundRobinPolicy {
		size_t index = 0;

		template<typename SessionWeakPtr>
		SessionWeakPtr select(std::vector<SessionWeakPtr>& sessions) {
			if (sessions.empty()) return SessionWeakPtr{};

			for (size_t i = 0; i < sessions.size(); ++i) {
				auto& w = sessions[index];
				index = (index + 1) % sessions.size();
				if (!w.expired())
					return w;
			}
			return SessionWeakPtr{}; // все сессии умерли
		}
	};

	template<typename SelectPolicy>
	class PgDataBase : public std::enable_shared_from_this<PgDataBase<SelectPolicy>> {
	public:
		PgDataBase(boost::asio::io_context& io, ConnectionInfo connection_info, SelectPolicy policy = SelectPolicy{}) :
			m_io_context(io),
			m_connection_info(std::move(connection_info)),
			m_connection_max(connection_info.max_sessions),
			m_strand(io.get_executor()),
			m_policy(std::move(policy))
		{

		}

		~PgDataBase()
		{
			terminate();
		}

		void connect_all()
		{
			for (unsigned i = 0; i < m_connection_max; ++i) {
				auto self = this->shared_from_this();
				boost::asio::co_spawn(
					m_strand,
					[self]() -> boost::asio::awaitable<void> {
						bool connected = co_await self->start_session();
					},
					boost::asio::detached
				);
			}
		}

		void connect_lazy()
		{
			auto self = this->shared_from_this();
			boost::asio::co_spawn(
				m_strand,
				[self]() -> boost::asio::awaitable<void> {
					bool connected = co_await self->start_session();
				},
				boost::asio::detached
			);
		}

		void terminate()
		{
			for (auto& session : m_sessions) {
				if (!session.expired()) {
					auto shared_ptr_session = session.lock();
					shared_ptr_session->close();
				}
			}
		}

		template<typename ResultEntity>
		boost::asio::awaitable<std::optional<std::vector<ResultEntity>>> async_execute(const std::string& query, std::shared_ptr<boost::system::error_code> ec) {
			try {
				if (query.empty() || m_sessions.empty())
					co_return std::nullopt;

				// Выбор сессии политикой класса
				auto session_weak = m_policy.select(m_sessions);

				if (session_weak.expired())
					co_return std::nullopt;

				auto session = session_weak.lock();

				// Работаем через strand
				co_await boost::asio::post(m_strand, boost::asio::use_awaitable);

				co_return co_await session->async_execute<ResultEntity>(query, ec);
			}
			catch (std::exception& ex) {
				spdlog::info("pg_data_base error when call execute query: {}", ex.what());
				co_return std::nullopt;
			}
		}

	private:
		boost::asio::awaitable<bool> start_session()
		{
			std::string con_info = connection_info_to_string();

			if (con_info.empty())
				co_return false;

			co_await boost::asio::post(m_strand, boost::asio::use_awaitable);

			m_sessions.erase(std::remove_if(m_sessions.begin(), m_sessions.end(),
				[](const std::weak_ptr<db::PgSession>& s) { return s.expired(); }),
				m_sessions.end());

			if (m_sessions.size() >= m_connection_max) {
				co_return false;
			}

			auto session = std::make_shared<db::PgSession>(m_io_context, con_info);
			session->start();

			m_sessions.emplace_back(session);

			co_return true;
		}

		std::string connection_info_to_string() const
		{
			return fmt::format(
				"dbname={} user={} password={} host={} port={} application_name={}",
				m_connection_info.dbname,
				m_connection_info.user,
				m_connection_info.password,
				m_connection_info.host,
				m_connection_info.port,
				m_connection_info.app_name
			);
		}

		boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

		SelectPolicy m_policy;

		std::vector<std::weak_ptr<PgSession>> m_sessions;
		boost::asio::io_context& m_io_context;
		unsigned m_connection_max = 1;
		ConnectionInfo m_connection_info;
	};
}