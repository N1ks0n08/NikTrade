#pragma once

#include <vector>
#include <deque>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <fmt/core.h>

// Utils & Logging
#include "utils/file_logger.hpp"

// UI
#include "ui/core/init.hpp"
#include "ui/windows/dataDisplay_window.hpp"
#include "ui/windows/orderBookDisplay_window.hpp"
#include "ui/windows/chartDisplay_window.hpp"
#include "ui/windows/banner_window.hpp"

// Core modules
#include "core/window_state.hpp"
#include "core/BBO.hpp"


class WindowManager {
public:
    WindowManager(FileLogger& logger, const std::vector<std::string>& symbols);
    ~WindowManager();

    bool shouldClose() const;
    void beginFrame();
    void render();
    void endFrame();

    std::vector<SymbolRequest>& pendingRequests();
    std::vector<WindowBBO>& activeWindows();

private:
    GLFWwindow* window_;
    std::vector<WindowBBO> activeBBOWindows_;
    std::vector<SymbolRequest> pendingRequests_;
};
