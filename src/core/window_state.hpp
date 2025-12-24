#pragma once
#include <string>

// Intent: what a window wants to view
struct SymbolRequest {
    int windowID;          // Unique ID of the window
    std::string requestedSymbol; // The symbol the window wants
    std::string requestType;    // Type of request ("stream", "close")
};

// Active window state: what symbol each window is currently assigned
struct WindowSymbol {
    int windowID;           // Unique ID of the window
    std::string activeSymbol;    // The currently active symbol
};
