#pragma once 
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <boost/asio/ip/tcp.hpp>

void init_async_logging(spdlog::level::level_enum level);