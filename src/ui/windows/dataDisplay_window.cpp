#include "dataDisplay_window.hpp"
#include <imgui.h>
#include <implot.h>
#include <vector>
#include <core/tech_indicators/sma.hpp>
#include <core/tech_indicators/ema.hpp>
#include <core/tech_indicators/rsi.hpp>
#include <core/tech_indicators/macd.hpp>
#include <core/tech_indicators/vwap.hpp>

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

        // Compute MACD, currently at:
        // Fast EMA: 12, Slow EMA: 26, Signal EMA: 9
        MACDResult macd_values = macdCalc(12, 26, 9, tickDataVector);

        // Compute VWAP
        std::vector<double> vwap_values = vwapCalc(tickDataVector);

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

        // Combined Subplots for Price, RSI, and MACD
        // -------------------------------
        if (ImPlot::BeginSubplots("SPY Indicator Dashboard", 3, 1, ImVec2(-1, 600),
            ImPlotSubplotFlags_LinkAllX | ImPlotSubplotFlags_NoResize)) {

            // ---------- PRICE PLOT ----------
            if (ImPlot::BeginPlot("Price", "Time", "Price", ImVec2(-1, 0))) {
                double yMax = *std::max_element(yValues.begin(), yValues.end());
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, tickDataVector.size() - 1, ImPlotCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, yMax, ImPlotCond_Once);

                int lookback = 10;

                // Plot SPY close (black)
                ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);
                ImPlot::PlotLine("SPY Close", xValues.data(), yValues.data(), currentFrame);

                // Plot SMA (red)
                if (!smaValues.empty() && currentFrame > lookback) {
                    size_t visibleCount = std::min<size_t>(smaValues.size(), currentFrame - lookback);
                    ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);
                    ImPlot::PlotLine("10-day SMA", &xValues[lookback], smaValues.data(), visibleCount);
                }

                // Plot EMA (blue)
                if (!emaValues.empty() && currentFrame > lookback) {
                    size_t visibleCount = std::min<size_t>(emaValues.size(), currentFrame - lookback);
                    ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
                    ImPlot::PlotLine("10-day EMA", &xValues[lookback], emaValues.data(), visibleCount);
                }

                // Plot VWAP (purple)
                if (!vwap_values.empty() && currentFrame > lookback) {
                    size_t visibleCount = std::min<size_t>(vwap_values.size(), currentFrame - lookback);
                    ImPlot::SetNextLineStyle(ImVec4(0.5f, 0.0f, 0.5f, 1.0f), 2.0f);
                    ImPlot::PlotLine("VWAP", &xValues[lookback], vwap_values.data(), visibleCount);
                }

                ImPlot::EndPlot();
            }

            // ---------- RSI PLOT ----------
            if (ImPlot::BeginPlot("RSI", nullptr, "RSI", ImVec2(-1, 0))) {
                int lookback = 10;
                if (!rsiValues.empty() && currentFrame > lookback) {
                    int data_size = std::min<int>(currentFrame - lookback, static_cast<int>(rsiValues.size() - lookback));
                    std::vector<double> x(data_size);
                    for (int i = 0; i < data_size; ++i) x[i] = lookback + i;

                    // RSI line
                    ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), 2.0f);
                    ImPlot::PlotLine("RSI", x.data(), rsiValues.data() + lookback, data_size);

                    // Thresholds (70 / 30)
                    std::vector<double> overbought(data_size, 70.0);
                    std::vector<double> oversold(data_size, 30.0);
                    ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
                    ImPlot::PlotLine("Overbought", x.data(), overbought.data(), data_size);
                    ImPlot::PopStyleColor();

                    ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 0.0f, 1.0f, 0.5f));
                    ImPlot::PlotLine("Oversold", x.data(), oversold.data(), data_size);
                    ImPlot::PopStyleColor();
                }
                ImPlot::EndPlot();
            }

            // ---------- MACD PLOT ----------
            if (ImPlot::BeginPlot("MACD", "Time", "MACD", ImVec2(-1, 0))) {
                const auto& macd = macd_values.macd;
                const auto& signal = macd_values.signal;
                const auto& hist = macd_values.histogram;
                size_t total_size = std::min({ macd.size(), signal.size(), hist.size() });

                int lookback = 10;
                if (total_size > 0 && currentFrame > lookback) {
                    int data_size = std::min<int>(currentFrame - lookback, static_cast<int>(total_size - lookback));

                    std::vector<double> x(data_size);
                    for (int i = 0; i < data_size; ++i)
                        x[i] = lookback + i;

                    // MACD Line
                    ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 2.0f);
                    ImPlot::PlotLine("MACD Line", x.data(), macd.data() + lookback, data_size);

                    // Signal Line
                    ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.2f, 1.0f, 1.0f), 2.0f);
                    ImPlot::PlotLine("Signal Line", x.data(), signal.data() + lookback, data_size);

                    // Histogram
                    ImPlot::PlotBars("Histogram", x.data(), hist.data() + lookback, data_size, 0.5);
                }

                ImPlot::EndPlot();
            }

            ImPlot::EndSubplots();
        }
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
}
