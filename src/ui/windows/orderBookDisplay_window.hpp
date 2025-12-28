#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "utils/file_logger.hpp"
#include "core/net/zmq_control_client.hpp"
#include <GLFW/glfw3.h>
#include "core/window_state.hpp"
#include "core/SymbolRequest.hpp"
#include "core/BBO.hpp"

void orderBookDisplayWindow(
    GLFWwindow* window,
    int windowWidth,
    int windowHeight,
    const std::vector<std::string>& availableSymbols,
    //const std::vector<uint8_t>& latestCryptoMessage,
    //ZMQControlClient& controlClient,
    FileLogger& logger,
    std::vector<SymbolRequest>& pendingRRequests,
    std::vector<WindowBBO>& activeWindows,
    int& windowID
);
