#include "dataDisplay_window.hpp"
#include <imgui.h>
#include <implot.h>
#include <vector>
#include <algorithm>
#include <fmt/core.h>

#include "ui/plots/price_plot.hpp"
#include "ui/plots/sma_plot.hpp"
#include "ui/plots/ema_plot.hpp"
#include "ui/plots/vwap_plot.hpp"
#include "ui/plots/rsi_plot.hpp"
#include "ui/plots/macd_plot.hpp"

#include "ui/backtest_plots/sma_crossover_plot.hpp" // UTILIZED TO PLOT THE SMA CROSSOVER BACKTEST VECTOR
#include "ui/backtest_plots/macd_vwapBacktester_plot.hpp" // UTILIZED TO PLOT THE MACD VWAP BACKTEST VECTOR

#include "core/backtest_engines/Trade.hpp"
#include "core/backtest_engines/sma_crossover.hpp" // UTILIZED TO GET THE SMA CROSSOVER BACKTEST VECTOR
#include "core/backtest_engines/macd_vwapBacktester.hpp" // UTILIZED TO GET THE GET TEH MACD VWAP BACKTEST VECTOR

void dataDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight, std::vector<Tick>& tickDataVector) {
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(900, 400), ImGuiCond_Once);

    // White background + dark text
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    if (ImGui::Begin("Equity Data Display", nullptr,
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

        int macd_vwap_backtest_fastEMAPeriod = 12;
        int macd_vwap_backtest_slowEMAPeriod = 26;
        int macd_vwap_backtest_signalPeriod = 9;
        double macd_vwap_backtest_startingCapital = 50000;
        // ------BACKTEST SECTION-------------
        std::vector<Trade> tradeVector = sma_crossover_result(fastSMAPeriod, slowSMAPeriod, startingCapital, tickDataVector);
        std::vector<Trade> macd_vwap_backtest_tradeVector = MACD_VWAPBacktestResultCalc(
            macd_vwap_backtest_fastEMAPeriod, macd_vwap_backtest_slowEMAPeriod, macd_vwap_backtest_signalPeriod
            , macd_vwap_backtest_startingCapital, tickDataVector
        );

        ImGui::Text(fmt::format("Ticker data size: {}", tickDataVector.size()).c_str());
        /* ------------------ DEBUGGING ----------------- 
        ImGui::Text("MACD + VWAP Backtest Trades:");
        ImGui::BeginChild("TradeLogScroll", ImVec2(0, 200), true); // scrollable area, 200px tall

        for (size_t i = 0; i < macd_vwap_backtest_tradeVector.size(); ++i) {
            const Trade& t = macd_vwap_backtest_tradeVector[i];
            ImGui::Text("[%zu] Date: %s | Type: %s | Shares: %d | Strike: %.2f | UnrealizedPnL: %.2f | RealizedPnL: %.2f",
                i, t.execution_date.c_str(), t.order_type.c_str(), t.shares,
                t.strike_price, t.unrealizedPnL, t.realizedPnL
            );
        }

        ImGui::EndChild();
        // ------------------DEBUGGING-------------------- */

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
        if (currentFrame < static_cast<int>(tickDataVector.size()))
            currentFrame++;
        lastUpdate = now;
    }
        // ------------------- ImPlot LEGEND STYLING ----------------
        ImPlotStyle& style = ImPlot::GetStyle();
        style.LegendPadding = ImVec2(10, 10);
        style.LegendInnerPadding = ImVec2(5,5);

        // LEGEND BOX BACKGROUND (only available in newer ImPlot versions)
        style.Colors[ImPlotCol_LegendBg] = ImVec4(1,1,1,1); // white background
        style.Colors[ImPlotCol_LegendBorder] = ImVec4(0,0,0,1); // optional black border
        style.Colors[ImPlotCol_LegendText] = ImVec4(0,0,0,1); // text color
        // ------------------- ImPlot LEGEND STYLING ----------------


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

                // Plot MACD VWAP Backtest Trades (BUY/SELL markers)
                plot_MACD_VWAPBacktester(tickDataVector, macd_vwap_backtest_tradeVector, currentFrame);

                ImPlot::EndPlot();
            }

            
            // ---------- RSI PLOT ----------
            int lookback = 10;
             // RSI parameters
            int rsi_interval = 10;

            plotRSI(tickDataVector, rsi_interval, currentFrame, lookback);            

            // Inside your ImPlot subplot for MACD
            if (ImPlot::BeginPlot("MACD", "Time", "MACD", ImVec2(-1, 0))) {
                int lookBack = 10;
                // MACD parameters
                int fastEMA_period = 12;
                int slowEMA_period = 26;
                int signal_period  = 9;

                // Plot MACD, Signal, Histogram dynamically
                plotMACD(tickDataVector, fastEMA_period, slowEMA_period, signal_period, currentFrame, lookBack);

                ImPlot::EndPlot();
            }
            ImPlot::EndSubplots();
        }
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
}
