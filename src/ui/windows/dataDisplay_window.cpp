#include "dataDisplay_window.hpp"
#include <imgui.h>
#include <vector>

void dataDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight, std::vector<Tick>& tickDataVector) {
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(900, 400), ImGuiCond_Once);

    // White background + dark text
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    if (ImGui::Begin("SPY 2025 EOD Data Graph", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Animated SPY Price Graph (YTD)");

        if (tickDataVector.empty()) {
            ImGui::Text("No tick data loaded.");
            ImGui::End();
            ImGui::PopStyleColor(2);
            return;
        }

        // Get drawlist + current window position
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImVec2(850, 300);
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y);

        // Draw background (white)
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(200, 200, 200, 255)); // grey border

        // Time-based frame update (¼ speed)
        static double lastUpdate = 0.0;
        static int currentFrame = 0;
        double now = ImGui::GetTime();

        // Update roughly every 1/15th second instead of every frame (~4x slower)
        if (now - lastUpdate > 0.066) { // 0.066s ≈ 15 FPS
            currentFrame = (currentFrame + 1) % (int)tickDataVector.size();
            lastUpdate = now;
        }

        // Compute max price for Y scaling
        double maxPrice = 0.0;
        for (const auto& tick : tickDataVector)
            if (tick.close > maxPrice) maxPrice = tick.close;

        // Draw line up to current frame
        const ImU32 lineColor = IM_COL32(46, 140, 230, 255); // same blue as title bar
        for (int i = 1; i < currentFrame; ++i) {
            float x0 = canvas_p0.x + (i - 1) * (canvas_size.x / (float)tickDataVector.size());
            float y0 = canvas_p1.y - (float)((tickDataVector[i - 1].close / maxPrice) * canvas_size.y);
            float x1 = canvas_p0.x + i * (canvas_size.x / (float)tickDataVector.size());
            float y1 = canvas_p1.y - (float)((tickDataVector[i].close / maxPrice) * canvas_size.y);

            draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), lineColor, 2.0f);
        }

        // Maintain spacing
        ImGui::Dummy(canvas_size);
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
}
