#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "utils/file_logger.hpp"
#include "core/net/zmq_control_client.hpp"
#include <GLFW/glfw3.h>

void cryptoDataDisplayWindow(
    GLFWwindow* window,
    int windowWidth,
    int windowHeight,
    const std::vector<std::string>& availableSymbols,
    const std::vector<uint8_t>& latestCryptoMessage,
    ZMQControlClient& controlClient,
    FileLogger& logger
);
