#include "cryptoDataDisplay_Window.hpp"
#include <imgui.h>
#include <implot.h>
#include <fmt/core.h>
#include <string>
#include <deque>

// Reads messages from a deque instead of directly from the concurrent queue.
void cryptoDataDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight,
                             const std::deque<const Binance::BookTicker*>& latestCryptoMessages) {

    // --------------------- Setup ImGui window ---------------------
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(900, 400), ImGuiCond_Once);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));    // white background
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f)); // dark text

    if (ImGui::Begin("Real-time Binance Crypto Data Display", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {

        ImGui::Text("Binance Crypto Tickers");

        // ------------------------------------------------------------------
        // Iterate through the deque of most recent BookTicker messages
        // (No popping or modification needed; deque is maintained in main.cpp)
        // ------------------------------------------------------------------
        for (const Binance::BookTicker* bookTicker : latestCryptoMessages) {
            if (!bookTicker) continue;

            double bid = 0.0, bidQty = 0.0, ask = 0.0, askQty = 0.0;

            if (bookTicker->best_bid()) bid = std::stod(bookTicker->best_bid()->c_str());
            if (bookTicker->bid_qty())  bidQty = std::stod(bookTicker->bid_qty()->c_str());
            if (bookTicker->best_ask()) ask = std::stod(bookTicker->best_ask()->c_str());
            if (bookTicker->ask_qty())  askQty = std::stod(bookTicker->ask_qty()->c_str());

            std::string msg = fmt::format(
                "{} | Bid: {:.4f} ({:.2f}) | Ask: {:.4f} ({:.2f})",
                bookTicker->symbol() ? bookTicker->symbol()->c_str() : "[null]",
                bid, bidQty, ask, askQty
            );

            ImGui::TextUnformatted(msg.c_str());
        }
    }

    // --------------------- Pop colors and end window ---------------------
    ImGui::PopStyleColor(2);
    ImGui::End();
}
