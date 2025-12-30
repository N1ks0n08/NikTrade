#include "banner_window.hpp"
#include <fmt/core.h>

namespace NikTrade {

void bannerWindow(
    bool& binanceConnected, 
    bool& zmqActive, 
    std::string& latency_message,
    std::vector<WindowBBO>& activeBBOWindows
) {
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 90)); // MAKE SURE TO OFFSET DOCKING SPACE IF NEEDED
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.10f, 0.25f, 1.0f));

    if (ImGui::Begin("NikTrade Banner",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse)) {
                     // | ImGuiWindowFlags_NoDocking)) {No need for this for now

        ImGui::SetCursorPosY(25);

        // ---- App name ----
        ImGui::TextColored(ImVec4(0.35f, 0.7f, 1.0f, 1.0f), "NikTrade");
        ImGui::SameLine(160);
        ImGui::Text("Real-Time Quant Dashboard");

        // ---- New BBO Window Button ----
        ImGui::SameLine(420);
        if (ImGui::Button("+ New BBO Window")) {
            bool created = false;

            // Reuse inactive slot
            for (auto& win : activeBBOWindows) {
                if (!win.active) {
                    win.active = true;
                    win.currentBBO = {};
                    created = true;
                    break;
                }
            }

            // Append new slot if allowed
            if (!created && activeBBOWindows.size() < activeBBOWindows.capacity()) {
                WindowBBO win;
                win.active = true;
                win.windowID = static_cast<int>(activeBBOWindows.size());
                activeBBOWindows.push_back(std::move(win));
                created = true;
            }

            // Optional: feedback if full
            if (!created) {
                // You can hook a toast / log here later
            }
        }

        // ---- Divider ----
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float lineX = io.DisplaySize.x * 0.4f;
        draw_list->AddLine(
            ImVec2(lineX, p.y + 5),
            ImVec2(lineX, p.y + 35),
            IM_COL32(200, 200, 255, 255),
            1.5f
        );

        // ---- Status indicators ----
        ImGui::SetCursorPosX(io.DisplaySize.x * 0.45f);

        ImGui::Text("Binance: ");
        ImGui::SameLine();
        ImGui::TextColored(
            binanceConnected ? ImVec4(0.1f, 0.9f, 0.3f, 1.0f)
                             : ImVec4(0.9f, 0.2f, 0.2f, 1.0f),
            binanceConnected ? "Connected" : "Offline"
        );

        ImGui::SameLine();
        ImGui::Text("|  ZMQ: ");
        ImGui::SameLine();
        ImGui::TextColored(
            zmqActive ? ImVec4(0.1f, 0.9f, 0.3f, 1.0f)
                      : ImVec4(0.9f, 0.2f, 0.2f, 1.0f),
            zmqActive ? "Active" : "Inactive"
        );

        ImGui::SameLine();
        ImGui::Text("| %s", latency_message.c_str());
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

} // namespace NikTrade