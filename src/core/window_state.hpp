#pragma once
#include <string>

using WindowID = int;

// Intent: what a window wants to view
struct SymbolRequest {
    WindowID windowID;          // Unique ID of the window
    std::string requestedSymbol; // The symbol the window wants
};

// Active window state: what symbol each window is currently assigned
struct WindowSymbol {
    WindowID windowID;           // Unique ID of the window
    std::string activeSymbol;    // The currently active symbol
};
