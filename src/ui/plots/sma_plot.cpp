#include "sma_plot.hpp"
#include <algorithm>

void plotSMA(const std::vector<Tick>& tickData, int period, int currentFrame, const char* label, ImVec4 color) {
    if (tickData.empty()) return;

    auto smaValues = smaCalc(period, tickData);
    std::vector<double> xValues(tickData.size());
    for (size_t i = 0; i < tickData.size(); ++i)
        xValues[i] = static_cast<double>(i + 1);

    if (!smaValues.empty() && currentFrame >= period) {
        size_t visibleCount = std::min(smaValues.size(), static_cast<size_t>(currentFrame - (period - 1)));
        if (visibleCount > 0) {
            ImPlot::SetNextLineStyle(color, 2.0f);
            ImPlot::PlotLine(label, &xValues[period - 1], smaValues.data(), visibleCount);
        }
    }
}
