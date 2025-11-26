#pragma once

#include <pqxx/pqxx>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include "pg_mapper.h"
#include <spdlog/spdlog.h>
#include <optional>

namespace db {

    class PgSession : public std::enable_shared_from_this<PgSession> {
    public:
        PgSession(boost::asio::io_context& ioc, std::string conn_str);
        ~PgSession();

        template<typename T>
        boost::asio::awaitable<std::optional<std::vector<T>>> async_execute(const std::string& query, std::shared_ptr<boost::system::error_code> ec) {
            ec->clear();
            std::vector<T> results;

            try {
                co_await boost::asio::post(m_strand, boost::asio::use_awaitable);

                if (*ec) {
                    co_return std::nullopt;
                }

                if (!is_open())
                    co_return std::nullopt;

                pqxx::work txn(m_connection.value());
                pqxx::result r = txn.exec(query);
                txn.commit();

                for (const auto& row : r) {
                    results.push_back(PgRowMapper<T>::map(row));
                }
            }
            catch (const pqxx::sql_error& e) {
                *ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);
                co_return std::nullopt;
            }
            catch (const std::exception& e) {
                *ec = boost::system::errc::make_error_code(boost::system::errc::operation_canceled);
                co_return std::nullopt;
            }

            co_return results;
        }

        void close();
        void start();
        bool is_open() const;

    private:
        void assign_socket();
        boost::asio::awaitable<void> wait();
        boost::asio::awaitable<bool> connect();
        boost::asio::awaitable<void> close_coro();

        boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

        std::string m_connection_str;
        std::optional<pqxx::connection> m_connection;
        boost::asio::io_context& m_io_context;
        boost::asio::ip::tcp::socket m_socket;
    };
}