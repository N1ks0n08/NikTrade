#include "macd_plot.hpp"
#include <fmt/core.h>

void plotMACD(const std::vector<Tick>& tickData, int& fastEMA_period, int& slowEMA_period, int& signal_period, int currentFrame, int lookback = 10) {
    MACDResult macd_values = macdCalc(fastEMA_period, slowEMA_period, signal_period, tickData);
    const auto& macd   = macd_values.macd;
    const auto& signal = macd_values.signal;
    const auto& hist   = macd_values.histogram;
    /*
    size_t total_size = std::min({ macd.size(), signal.size(), hist.size() });
    if (total_size == 0 || currentFrame <= lookback)
        return;

    int data_size = std::min<int>(currentFrame - lookback, static_cast<int>(total_size - lookback));

    std::vector<double> x(data_size);
    for (int i = 0; i < data_size; ++i)
        x[i] = lookback + i + 1;
    
    ImGui::Text(fmt::format("MACD size: {}\nSignal size: {}", macd_values.macd.size(), macd_values.signal.size()).c_str());

    // --- MACD Line ---
    if (macd.size() >= lookback + data_size) {
        ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 2.0f);
        ImPlot::PlotLine("MACD Line", x.data(), macd.data() + lookback, data_size);
    }

    // --- Signal Line ---
    if (signal.size() >= lookback + data_size) {
        ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.2f, 1.0f, 1.0f), 2.0f);
        ImPlot::PlotLine("Signal Line", x.data(), signal.data() + lookback, data_size);
    }

    // --- Histogram ---
    if (hist.size() >= lookback + data_size) {
        ImPlot::PlotBars("Histogram", x.data(), hist.data() + lookback, data_size, 0.5);
    } */
        // Sanity check
    if (macd.empty() || signal.empty() || hist.empty()) {
        ImGui::Text("MACD, Signal, or Histogram vector is empty!");
        return;
    }

    // Prepare X-axis (just use index)
    size_t total_size = macd.size(); // static plotting uses full MACD vector
    std::vector<double> x(total_size);
    for (size_t i = 0; i < total_size; ++i) {
        x[i] = static_cast<double>(i);
    }

    ImGui::Text(fmt::format("MACD size: {}, Signal size: {}, Hist size: {}",
                            macd.size(), signal.size(), hist.size()).c_str());

    // --- MACD Line ---
    ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 2.0f);
    ImPlot::PlotLine("MACD Line", x.data(), macd.data(), total_size);

    // --- Signal Line ---
    // Align with MACD start if lengths differ
    size_t signal_offset = macd.size() - signal.size();
    ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.2f, 1.0f, 1.0f), 2.0f);
    ImPlot::PlotLine("Signal Line", x.data() + signal_offset, signal.data(), signal.size());

    // --- Histogram ---
    size_t hist_offset = macd.size() - hist.size();
    ImPlot::PlotBars("Histogram", x.data() + hist_offset, hist.data(), hist.size(), 0.5f);
}
