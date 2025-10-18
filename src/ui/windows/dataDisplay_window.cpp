#include "dataDisplay_window.hpp"
#include <imgui.h>
#include <implot.h>
#include <vector>
#include <core/tech_indicators/sma.hpp>
#include <core/tech_indicators/ema.hpp>
#include <core/tech_indicators/rsi.hpp>

void dataDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight, std::vector<Tick>& tickDataVector) {
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(900, 400), ImGuiCond_Once);

    // White background + dark text
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    if (ImGui::Begin("SPY 2025 EOD Data Graph", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings))
        // | ImGuiWindowFlags_AlwaysAutoResize)) Not in use for now
    {
        ImGui::Text("Animated SPY Price Graph (YTD)");

        if (tickDataVector.empty()) {
            ImGui::Text("No tick data loaded.");
            ImGui::End();
            ImGui::PopStyleColor(2);
            return;
        }

        // Compute SMA, currently 10 day interval
        std::vector<double> smaValues = smaCalc(10, tickDataVector);

        // Compute EMA, currently 10 day interval
        std::vector<double> emaValues = emaCalc(10, tickDataVector);

        // Compute RSI, currently 10 day interval
        std::vector<double> rsiValues = rsiCalc(10, tickDataVector);

        // -------------------------------
        // Prepare X/Y arrays for ImPlot
        // -------------------------------
        std::vector<double> xValues(tickDataVector.size());
        std::vector<double> yValues(tickDataVector.size());
        for (size_t i = 0; i < tickDataVector.size(); ++i) {
            xValues[i] = static_cast<double>(i);
            yValues[i] = tickDataVector[i].close;
        }

        // Time-based frame update (¼ speed)
        static double lastUpdate = 0.0;
        static int currentFrame = 0;
        double now = ImGui::GetTime();

        if (now - lastUpdate > 0.066) { // ~15 FPS
            currentFrame = (currentFrame + 1) % (int)tickDataVector.size();
            lastUpdate = now;
        }

        // -------------------------------
        // Plot with ImPlot:
        // Price + SMA + EMA + RSI
        // -------------------------------
        if (ImPlot::BeginPlot("Price Plot", "Time", "Price", ImVec2(-1, 300))) {
            double yMax = *std::max_element(yValues.begin(), yValues.end());

            // Constrain the X axis: can’t scroll left of 0, but user can zoom/pan right freely
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, tickDataVector.size() - 1, ImPlotCond_Always);

            // Y axis: start at 0, allow zoom/pan up, but clamp minimum to 0
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, yMax, ImPlotCond_Once);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0, FLT_MAX); // 0 = min, FLT_MAX = no upper limit

            // DO NOT call SetupAxesLimits() or SetupFinish() here.
            // Those are deprecated and cause the PopID() imbalance.

            int lookback = 10;

            // Plot SPY close (black line)
            ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);
            ImPlot::PlotLine("SPY Close", xValues.data(), yValues.data(), currentFrame);

            // Plot SMA (red line)
            if (!smaValues.empty() && currentFrame > lookback) {
                ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);
                size_t visibleCount = std::min<size_t>(smaValues.size(), currentFrame - lookback);
                ImPlot::PlotLine("10-day SMA", &xValues[lookback], smaValues.data(), visibleCount);
            }

            // Plot EMA (blue line)
            if (!emaValues.empty() && currentFrame > lookback) {
                ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
                size_t visibleCount = std::min<size_t>(emaValues.size(), currentFrame - lookback);
                ImPlot::PlotLine("10-day EMA", &xValues[lookback], emaValues.data(), visibleCount);
            }

            // Plot RSI (green line)
            if (!rsiValues.empty() && currentFrame > lookback) {
                ImPlot::SetNextLineStyle(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 2.0f);
                size_t visibleCount = std::min<size_t>(rsiValues.size(), currentFrame - lookback);
                ImPlot::PlotLine("10-day RSI", &xValues[lookback], rsiValues.data(), visibleCount);
            }

            ImPlot::EndPlot(); // always must match BeginPlot
        }
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
}
