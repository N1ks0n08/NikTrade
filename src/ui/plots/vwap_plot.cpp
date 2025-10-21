#include "vwap_plot.hpp"
#include <algorithm>

void plotVWAP(const std::vector<Tick>& tickData, int currentFrame, const char* label, ImVec4 color) {
    if (tickData.empty()) return;

    auto vwapValues = vwapCalc(tickData);
    std::vector<double> xValues(tickData.size());
    for (size_t i = 0; i < tickData.size(); ++i)
        xValues[i] = static_cast<double>(i + 1);

    if (!vwapValues.empty() && currentFrame > 0) {
        size_t visibleCount = std::min(vwapValues.size(), static_cast<size_t>(currentFrame));
        if (visibleCount > 0) {
            ImPlot::SetNextLineStyle(color, 2.0f);
            ImPlot::PlotLine(label, xValues.data(), vwapValues.data(), visibleCount);
        }
    }
}
