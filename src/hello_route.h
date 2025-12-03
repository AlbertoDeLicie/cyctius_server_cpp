#pragma once
#include <boost/asio.hpp>
#include "core/common/http_response.h"
#include "core/common/http_request.h"
#include "core/common/websocket_message.h"
#include "core/db/pg_data_base.h"
#include "message.h"

class HelloRoute {
public:
    HelloRoute(boost::asio::io_context& io_context);

    boost::asio::awaitable<std::shared_ptr<RawResponse>> hello_json(std::shared_ptr<HttpRequest<JsonContent>> req);
    boost::asio::awaitable<std::shared_ptr<WebsocketRawMessage>> ws_hello_json(std::shared_ptr<Message> msg);

private:
    boost::asio::io_context& m_io_context;
    std::shared_ptr<db::PgDataBase<db::RoundRobinPolicy>> m_db;
};
