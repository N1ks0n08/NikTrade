#include "ema_plot.hpp"
#include <algorithm>

void plotEMA(const std::vector<Tick>& tickData, int period, int currentFrame, const char* label, ImVec4 color) {
    if (tickData.empty()) return;

    auto emaValues = emaCalc(period, tickData);
    std::vector<double> xValues(tickData.size());
    for (size_t i = 0; i < tickData.size(); ++i)
        xValues[i] = static_cast<double>(i + 1);

    if (!emaValues.empty() && currentFrame >= period) {
        size_t visibleCount = std::min(emaValues.size(), static_cast<size_t>(currentFrame - (period - 1)));
        if (visibleCount > 0) {
            ImPlot::SetNextLineStyle(color, 2.0f);
            ImPlot::PlotLine(label, &xValues[period - 1], emaValues.data(), visibleCount);
        }
    }
}
