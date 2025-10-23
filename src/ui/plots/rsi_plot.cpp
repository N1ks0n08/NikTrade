#include "rsi_plot.hpp"
#include <fmt/core.h>

void plotRSI(const std::vector<Tick>& tickData, int& rsi_interval, int& currentFrame, int& lookBack) {
    if (tickData.empty() || rsi_interval <= 0)
        return;

    // Compute RSI values
    std::vector<double> rsiValues = rsiCalc(rsi_interval, tickData);
    if (rsiValues.empty())
        return;

    // --- Static full-plot version (ignore currentFrame / lookBack) ---
    int data_size = static_cast<int>(rsiValues.size());
    // RSI Starting point
    int starting_xpos_RSI = rsi_interval;
    std::vector<double> x(data_size);
    for (int i = 0; i < data_size; ++i)
        x[i] = static_cast<double>(i + starting_xpos_RSI);

    //ImGui::Text(fmt::format("RSI size: {}", data_size).c_str());

    if (ImPlot::BeginPlot("RSI", "Time", "RSI", ImVec2(-1, 0))) {
        // RSI line
        ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), 2.0f);
        ImPlot::PlotLine("RSI", x.data(), rsiValues.data(), data_size);

        // Overbought (70) and oversold (30) threshold lines
        std::vector<double> overbought(data_size, 70.0);
        std::vector<double> oversold(data_size, 30.0);

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
        ImPlot::PlotLine("Overbought", x.data(), overbought.data(), data_size);
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 0.0f, 1.0f, 0.5f));
        ImPlot::PlotLine("Oversold", x.data(), oversold.data(), data_size);
        ImPlot::PopStyleColor();

        ImPlot::EndPlot();
    }
}