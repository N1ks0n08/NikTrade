#pragma once
#include <string>
#include "core/BBO.hpp"

// Intent: what a window wants to view
struct SymbolRequest {
    int windowID;          // Unique ID of the window
    std::string requestedSymbol; // The symbol the window wants
    std::string requestType;    // Type of request ("stream", "close")
};

// Active window state: what symbol each window is currently assigned
struct WindowBBO {
    int windowID;           // Unique ID of the window
    BBO currentBBO;      // Current BBO data for the window
};
