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

    bool windowOpen = ImGui::Begin("Orderbook Display", nullptr, ImGuiWindowFlags_NoCollapse);
    if (!windowOpen) {
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
    static char searchBuf[32] = "";
    bool enterPressed = ImGui::InputText("Symbol", searchBuf, IM_ARRAYSIZE(searchBuf), ImGuiInputTextFlags_EnterReturnsTrue);

    auto isValidSymbol = [&](const std::string& s) {
        return std::find(availableSymbols.begin(), availableSymbols.end(), s) != availableSymbols.end();
    };
    /*
    auto trySendChange = [&]() {
        std::string symbol(searchBuf);
        if (!isValidSymbol(symbol)) {
            logger.logInfo("[WARN] Invalid symbol, ignoring request.");
            return;
        }

        std::string reply;
        bool ok = controlClient.sendControlRequest(
            fmt::format("switch_symbol {}", symbol),
            reply,
            logger,
            500
        );

        if (ok) logger.logInfo(fmt::format("[INFO] Switched symbol: {}", reply));
        else logger.logInfo("[WARN] Failed to switch symbol.");
    }; */

    auto trySendChange = [&]() {
        std::string symbol(searchBuf);
        if (!isValidSymbol(symbol)) {
            logger.logInfo("[WARN] Invalid symbol, ignoring request.");
            return;
        }

        // Create and store the symbol request
        SymbolRequest req;
        req.windowID = windowID;
        req.requestedSymbol = symbol;
        req.requestType = "stream";

        pendingRRequests.emplace_back(req);
    };

    ImGui::SameLine();

    // Implement 1.5 second cooldown for the button LATER
    if (ImGui::Button("Select")) { trySendChange(); }
    if (enterPressed) { trySendChange(); }

    // -------------------------
    // Symbol List
    // -------------------------
    if (ImGui::BeginChild("symbols", ImVec2(200, 200), true)) {
        for (const auto& sym : availableSymbols) {
            if (ImGui::Selectable(sym.c_str())) {
                strcpy(searchBuf, sym.c_str()); // autofill
            }
        }
    }
    ImGui::EndChild();
    ImGui::Dummy(ImVec2(0, 15));
    
    /*
    // -------------------------
    // Live Ticker Decode
    // -------------------------
    if (!latestCryptoMessage.empty()) {
        logger.logInfo("Message successfully received for display.");
        const Binance::BookTicker* ticker = Binance::GetBookTicker(latestCryptoMessage.data());
        if (ticker) {
            auto safe = [](const flatbuffers::String* s) -> double {
                if (!s || s->str().empty()) return 0.0;
                try { return std::stod(s->str()); } catch (...) { return 0.0; }
            };

            const char* symbol = ticker->symbol() ? ticker->symbol()->c_str() : "[null]";
            logger.logInfo(fmt::format("Displaying ticker for symbol: {}", symbol));
            double bid = safe(ticker->best_bid());
            double bidQty = safe(ticker->bid_qty());
            double ask = safe(ticker->best_ask());
            double askQty = safe(ticker->ask_qty());

            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", symbol);
            ImGui::SameLine();
            ImGui::Text("Bid: %.4f (%.2f) | Ask: %.4f (%.2f)", bid, bidQty, ask, askQty);
        } else {
            ImGui::Text("Invalid FlatBuffer data received.");
        }
    } else {
        ImGui::Text("Waiting for live data...");
    }
    */

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