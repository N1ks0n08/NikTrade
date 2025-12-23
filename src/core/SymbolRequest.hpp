#pragma once
#include "utils/file_logger.hpp"
#include "core/net/zmq_control_client.hpp"
#include <fmt/core.h>

void handleSymbolChangeRequest(
    const std::string& symbol,
    ZMQControlClient& controlClient,
    FileLogger& logger
);
