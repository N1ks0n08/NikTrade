#include "dataDisplay_window.hpp"
#include <imgui.h>
#include <implot.h>
#include <vector>
#include <algorithm>
#include "ui/plots/price_plot.hpp"
#include "ui/plots/sma_plot.hpp"
#include "ui/plots/ema_plot.hpp"
#include "ui/plots/vwap_plot.hpp"
#include "ui/backtest_plots/sma_crossover_plot.hpp" // UTILIZED TO PLOT THE SMA CROSSOVER BACKTEST VECTOR
#include "core/tech_indicators/rsi.hpp"
#include "core/tech_indicators/macd.hpp"
#include "core/backtest_engines/Trade.hpp"
#include "core/backtest_engines/sma_crossover.hpp" // UTILIZED TO GET THE SMA CROSSOVER BACKTEST VECTOR

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


        // ------BACKTEST SECTION ------------
        int fastSMAPeriod = 10;
        int slowSMAPeriod = 50;
        double startingCapital = 50000;
        std::vector<double> slowSMAValues = smaCalc(slowSMAPeriod, tickDataVector);
        // ------BACKTEST SECTION-------------
        std::vector<Trade> tradeVector = sma_crossover_result(fastSMAPeriod, slowSMAPeriod, startingCapital, tickDataVector);

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
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        float rowRatios[] = { 0.6f, 0.2f, 0.2f };
        if (ImPlot::BeginSubplots("SPY Indicator Dashboard", 3, 1, ImVec2(windowSize.x, windowSize.y),
            0, nullptr, rowRatios)) {
            // ---------- PRICE + FAST/SLOW SMA + EMA + VWAP --------------//
            if (ImPlot::BeginPlot("Price", "Time", "Price", ImVec2(-1, 0))) {

                // Plot the stock price
                plotPrice(tickDataVector, currentFrame);
                
                // Plot SMAs
                plotSMA(tickDataVector, 10, currentFrame, "10-day SMA", ImVec4(1,0,0,1));
                plotSMA(tickDataVector, 50, currentFrame, "50-day SMA", ImVec4(1,0.5f,0,1));

                // Plot EMA
                plotEMA(tickDataVector, 10, currentFrame, "EMA", ImVec4(0,0,1,1));

                // Plot VWAP
                plotVWAP(tickDataVector, currentFrame, "VWAP", ImVec4(0.5f,0,0.5f,1));

                // Plot SMA Crossover Trades (BUY/SELL markers)
                plotSMACrossoverTrades(tickDataVector, tradeVector, currentFrame);

                ImPlot::EndPlot();
            }

            // ---------- RSI PLOT ----------
            if (ImPlot::BeginPlot("RSI", nullptr, "RSI", ImVec2(-1, 0))) {
                int lookback = 10;
                if (!rsiValues.empty() && currentFrame > lookback) {
                    int data_size = std::min<int>(currentFrame - lookback, static_cast<int>(rsiValues.size() - lookback));
                    std::vector<double> x(data_size);
                    for (int i = 0; i < data_size; ++i) x[i] = lookback + i + 1;

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
                        x[i] = lookback + i + 1;

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
