#include "cryptoChartDisplay_window.hpp"
#include <inttypes.h>
#include <imgui.h>
#include <implot.h>
#include <vector>
#include <deque>
#include <algorithm>

void cryptoChartDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight,
    const std::deque<KlineData>& klines)
{
    ImGui::Begin("Crypto Chart Display");

    if (klines.empty()) {
        ImGui::Text("No candles available.");
        ImGui::BeginChild("CandleScroll", ImVec2(0, 200), true);
        ImGui::Text("No candles available.");
        ImGui::EndChild();
        ImGui::End();
        return;
    }

    // Raw data display
    if (ImGui::CollapsingHeader("Raw Candle Data")) {
        ImGui::BeginChild("CandleScroll", ImVec2(0, 200), true);
        for (const auto& k : klines) {
            ImGui::Text("OpenTime: %" PRIu64 " | O: %.2f H: %.2f L: %.2f C: %.2f V: %.2f | CloseTime: %" PRIu64,
                        k.open_time, k.open, k.high, k.low, k.close, k.volume, k.close_time);
        }
        ImGui::EndChild();
    }

    const float halfWidth = 0.35f;

    if (ImPlot::BeginPlot("Candles", ImVec2(-1, -1))) {
        ImPlot::SetupAxis(ImAxis_X1, "Index", ImPlotAxisFlags_None);
        ImPlot::SetupAxisFormat(ImAxis_X1, "%.0f");
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, INFINITY);

        ImPlot::SetupAxis(ImAxis_Y1, "Price", ImPlotAxisFlags_None);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, INFINITY);

        int total = static_cast<int>(klines.size());
        ImPlot::SetupAxesLimits(0.0, static_cast<double>(total), 0.0, 100.0, ImPlotCond_Once);

        ImPlot::PushPlotClipRect();
        ImDrawList* drawList = ImPlot::GetPlotDrawList();

        for (int i = 0; i < total; ++i) {
            const KlineData& k = klines[i];
            float x = static_cast<float>(i);

            ImU32 color = (k.close > k.open) ? IM_COL32(0, 255, 0, 255) :
                           (k.close < k.open) ? IM_COL32(255, 0, 0, 255) :
                                                IM_COL32(180, 180, 180, 255);

            ImVec2 pHigh  = ImPlot::PlotToPixels(ImVec2(x, static_cast<float>(k.high)));
            ImVec2 pLow   = ImPlot::PlotToPixels(ImVec2(x, static_cast<float>(k.low)));
            ImVec2 pOpen  = ImPlot::PlotToPixels(ImVec2(x - halfWidth, static_cast<float>(k.open)));
            ImVec2 pClose = ImPlot::PlotToPixels(ImVec2(x + halfWidth, static_cast<float>(k.close)));

            drawList->AddLine(pHigh, pLow, color, 1.5f);
            drawList->AddRectFilled(pOpen, pClose, color);
            drawList->AddRect(pOpen, pClose, IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
        }

        ImPlot::PopPlotClipRect();
        ImPlot::EndPlot();
    }

    ImGui::End();
}
