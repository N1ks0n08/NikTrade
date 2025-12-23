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
    const std::vector<uint8_t>& latestCryptoMessage,
    ZMQControlClient& controlClient,
    FileLogger& logger
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
    };

    ImGui::SameLine();
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

    // -------------------------
    // Live Ticker Display
    // -------------------------
    

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

    ImGui::PopStyleColor(2);
    ImGui::End();
}