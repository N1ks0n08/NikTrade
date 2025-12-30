#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include "core/window_state.hpp"

namespace NikTrade {

    void bannerWindow(
        bool& binanceConnected, 
        bool& zmqActive, 
        std::string& latencyMs,
        std::vector<WindowBBO>& activeBBOWindows
    );
}
