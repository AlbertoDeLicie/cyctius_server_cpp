#pragma once
#include "logger.h"

void init_async_logging(spdlog::level::level_enum level) {
    spdlog::init_thread_pool(8192, 1);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto logger = std::make_shared<spdlog::logger>(
        "console",
        console_sink
    );

    logger->set_level(level);
    spdlog::set_default_logger(logger);
}
