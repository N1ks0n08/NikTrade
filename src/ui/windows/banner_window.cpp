#include "banner_window.hpp"
#include <fmt/core.h>

namespace NikTrade {

void bannerWindow(bool binanceConnected, bool zmqActive, std::string latency_message) {
    ImGuiIO& io = ImGui::GetIO();

    // Position at top of screen, full width
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 90));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // Futuristic deep blue background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.10f, 0.25f, 1.0f));

    if (ImGui::Begin("NikTrade Banner",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoDocking)) {

        ImGui::SetCursorPosY(25);

        // ---- Left side: App name ----
        ImGui::TextColored(ImVec4(0.35f, 0.7f, 1.0f, 1.0f), "NikTrade");
        ImGui::SameLine(250);
        ImGui::Text("Real-Time Quant Dashboard");

        // ---- Vertical line divider ----
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float lineX = io.DisplaySize.x * 0.4f;
        draw_list->AddLine(ImVec2(lineX, p.y + 5), ImVec2(lineX, p.y + 35), IM_COL32(200, 200, 255, 255), 1.5f);

        // ---- Right side: status indicators ----
        ImGui::SetCursorPosX(io.DisplaySize.x * 0.45f);

        ImGui::Text("Binance: ");
        ImGui::SameLine();
        ImGui::TextColored(binanceConnected ? ImVec4(0.1f, 0.9f, 0.3f, 1.0f)
                                            : ImVec4(0.9f, 0.2f, 0.2f, 1.0f),
                           binanceConnected ? "Connected" : "Offline");

        ImGui::SameLine();
        ImGui::Text("|  ZMQ: ");
        ImGui::SameLine();
        ImGui::TextColored(zmqActive ? ImVec4(0.1f, 0.9f, 0.3f, 1.0f)
                                     : ImVec4(0.9f, 0.2f, 0.2f, 1.0f),
                           zmqActive ? "Active" : "Inactive");

        ImGui::SameLine();
        ImGui::Text("%s", fmt::format("|  {}", latency_message).c_str());
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

}
