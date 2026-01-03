#include "orderBookDisplay_window.hpp"
#include <imgui.h>
#include <fmt/core.h>
#include <vector>
#include <string>
#include <algorithm>
#include "../../core/flatbuffers/Binance/binance_bookticker_generated.h"
#include "../../core/net/zmq_control_client.hpp"

void orderBookDisplayWindow(
    GLFWwindow* window,
    int windowWidth,
    int windowHeight,
    const std::vector<std::string>& availableSymbols,
    //const std::vector<uint8_t>& latestCryptoMessage,
    //ZMQControlClient& controlClient,
    FileLogger& logger,
    std::vector<SymbolRequest>& pendingRRequests,
    std::vector<WindowBBO>& activeBBOWindows,
    int& windowID
) {

    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(900, 350), ImGuiCond_Once);

    // Track window open state
    bool windowOpen = true;
    if (windowID < activeBBOWindows.size()) {
        windowOpen = activeBBOWindows[windowID].active;
    }

    // Begin ImGui window with close button support
    if (!ImGui::Begin(("Orderbook Display##" + std::to_string(windowID)).c_str(), &windowOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // If user clicked the 'X', mark window inactive and return
    if (!windowOpen) {
        if (windowID < activeBBOWindows.size()) {
            activeBBOWindows[windowID].active = false;

            // Tell backend to stop the stream
            SymbolRequest req;
            req.windowID = windowID;
            req.requestedSymbol = activeBBOWindows[windowID].desiredSymbol;
            req.requestType = "close_stream";

            pendingRRequests.emplace_back(req);
        }

        ImGui::End();
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.90f, 0.90f, 1.0f));

    ImGui::Text("Live Crypto BookTicker");
    ImGui::Separator();

    // -------------------------
    // Symbol Search Box
    // -------------------------
    // Per-window search buffer
    static std::vector<std::array<char, 32>> searchBuffers;
    if (searchBuffers.size() <= windowID) searchBuffers.resize(windowID + 1);
    auto& searchBuf = searchBuffers[windowID];

    // Unique label per window
    bool enterPressed = ImGui::InputText(("Symbol##" + std::to_string(windowID)).c_str(), searchBuf.data(), searchBuf.size(), ImGuiInputTextFlags_EnterReturnsTrue);


    auto isValidSymbol = [&](const std::string& s) {
        return std::find(availableSymbols.begin(), availableSymbols.end(), s) != availableSymbols.end();
    };
    
    auto trySendChange = [&]() {
        std::string symbol(searchBuf.data());
        if (!isValidSymbol(symbol)) {
            logger.logInfo("[WARN] Invalid symbol, ignoring request.");
            return;
        }

        // Create and store the symbol request
        SymbolRequest req;
        req.windowID = windowID;
        req.requestedSymbol = symbol;
        req.requestType = "start_stream";

        pendingRRequests.emplace_back(req);
    };

    ImGui::SameLine();

    // Implement 1.5 second cooldown for the button LATER
    if (ImGui::Button(("Select##" + std::to_string(windowID)).c_str())) {
        trySendChange(); 
    }
    if (enterPressed) { trySendChange(); }

    // -------------------------
    // Symbol List
    // -------------------------
    if (ImGui::BeginChild(("symbols##" + std::to_string(windowID)).c_str(), ImVec2(200, 200), true)) {
        for (size_t i = 0; i < availableSymbols.size(); ++i) {
            const auto& sym = availableSymbols[i];
            if (ImGui::Selectable((sym + "##" + std::to_string(windowID) + "_" + std::to_string(i)).c_str())) {
                strncpy(searchBuf.data(), sym.c_str(), searchBuf.size() - 1);
                searchBuf[searchBuf.size() - 1] = '\0';
            }
        }
        ImGui::EndChild();
    }
    ImGui::Dummy(ImVec2(0, 15));

    // -------------------------
    // Live Data Display
    // -------------------------
    if (windowID < activeBBOWindows.size()) {
        if (!activeBBOWindows[windowID].currentBBO.error.empty()) {
            ImGui::Text("Error retrieving BBO data: %s", activeBBOWindows[windowID].currentBBO.error.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", activeBBOWindows[windowID].currentBBO.symbol.c_str());
            ImGui::SameLine();
            ImGui::Text("Bid: %.4f (%.2f) | Ask: %.4f (%.2f)",
                activeBBOWindows[windowID].currentBBO.bid_price,
                activeBBOWindows[windowID].currentBBO.bid_quantity,
                activeBBOWindows[windowID].currentBBO.ask_price,
                activeBBOWindows[windowID].currentBBO.ask_quantity
            );
        }
    } else {
        logger.logInfo("No active BBO data for this window. (Does this index exist?)");
        ImGui::Text("Waiting for live data...");
    }

    ImGui::PopStyleColor(2);
    ImGui::End();
}