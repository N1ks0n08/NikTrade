#include "main_window.hpp"
#include <imgui.h>

/* Main dashboard window rendering logic */
void mainWindow(GLFWwindow* window, int windowWidth, int windowHeight) {
    // Configure the window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f);       // Round corners
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);      // No borders
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.12f, 1.0f)); // Slightly darker background
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));
    ImGui::Begin("mainWindow", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus   // <-- prevents it from covering other windows
    );

    // Get positions and sizes
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();

    // TITLE BAR LOGIC
    {
        // Decorative bar dimensions
        float barHeight = 30.0f;
        ImVec4 barColor = ImVec4(0.18f, 0.55f, 0.90f, 1.0f);
        ImU32 barCol = ImGui::ColorConvertFloat4ToU32(barColor);

        // Draw custom decorative bar (top area)
        draw_list->AddRectFilled(
            win_pos,
            ImVec2(win_pos.x + win_size.x, win_pos.y + barHeight),
            barCol
        );

        // Optional: title text on the bar
        ImGui::SetCursorScreenPos(ImVec2(win_pos.x + 15, win_pos.y + 6));
        ImGui::TextUnformatted("NikTrade Dashboard");

        // Offset content below the bar
        ImGui::SetCursorScreenPos(ImVec2(win_pos.x + ImGui::GetStyle().WindowPadding.x,
                                        win_pos.y + barHeight + ImGui::GetStyle().WindowPadding.y));
    }

    // Example content
    ImGui::Text("Window content here...");
    ImGui::Dummy(ImVec2(0.0f, 150.0f));
    ImGui::Text("Rounded corners + custom bar ✅");

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}
