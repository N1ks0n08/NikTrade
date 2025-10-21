#include "price_plot.hpp"
#include <algorithm>
#include <implot.h>

void plotPrice(const std::vector<Tick>& tickDataVector, int currentFrame) {
    if (tickDataVector.empty())
        return;

    // Prepare X/Y arrays
    std::vector<double> xValues(tickDataVector.size());
    std::vector<double> yValues(tickDataVector.size());

    for (size_t i = 0; i < tickDataVector.size(); ++i) {
        xValues[i] = static_cast<double>(i + 1);
        yValues[i] = tickDataVector[i].close;
    }

    double yMin = *std::min_element(yValues.begin(), yValues.end());
    double yMax = *std::max_element(yValues.begin(), yValues.end());

    // Only initialize axis limits once — allow zooming/panning afterward
    ImPlot::SetupAxisLimits(ImAxis_X1, 0, tickDataVector.size() - 1, ImPlotCond_Once);
    ImPlot::SetupAxisLimits(ImAxis_Y1, yMin * 0.98, yMax * 1.02, ImPlotCond_Once);

    // Plot the stock price line (black, 2px)
    ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, 1), 2.0f);
    ImPlot::PlotLine("Stock Price", xValues.data(), yValues.data(), currentFrame);
}
