#include "cryptoDataDisplay_window.hpp"
#include <imgui.h>
#include <implot.h>
#include <fmt/core.h>
#include <vector>
#include <deque>
#include <flatbuffers/flatbuffers.h>
#include "../../core/flatbuffers/Binance/binance_bookticker_generated.h"

void cryptoDataDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight,
                            const std::deque<std::vector<uint8_t>>& latestCryptoMessages) 
{
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(900, 400), ImGuiCond_Once);

    bool windowOpen = ImGui::Begin("Crypto Data Display", nullptr,
                                  ImGuiWindowFlags_NoCollapse);
    if (!windowOpen) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));

    ImGui::Text("Binance Crypto Tickers (Live Updates)");
    ImGui::Separator();
    ImGui::Text("latestCryptoMessages size = %d", (int)latestCryptoMessages.size());
    for (const auto& msg : latestCryptoMessages) {
        if (msg.empty()) {
            continue;
        }

        // Decode FlatBuffer
        const Binance::BookTicker* bookTicker = Binance::GetBookTicker(msg.data());

        // Extract values safely
        auto safeToDouble = [](const flatbuffers::String* s) -> double {
            if (!s || s->str().empty()) return 0.0;
            try { return std::stod(s->str()); }
            catch (...) { return 0.0; }
        };

        double bid = safeToDouble(bookTicker->best_bid());
        double bidQty = safeToDouble(bookTicker->bid_qty());
        double ask = safeToDouble(bookTicker->best_ask());
        double askQty = safeToDouble(bookTicker->ask_qty());
        const char* symbol = bookTicker->symbol() ? bookTicker->symbol()->c_str() : "[null]";

        ImGui::TextColored(ImVec4(0.0f, 0.6f, 0.0f, 1.0f), "▲ %s", symbol);
        ImGui::SameLine();
        ImGui::Text("Bid: %.4f (%.2f) | Ask: %.4f (%.2f)", bid, bidQty, ask, askQty);
    }
    ImGui::PopStyleColor(2);
    ImGui::End();
}