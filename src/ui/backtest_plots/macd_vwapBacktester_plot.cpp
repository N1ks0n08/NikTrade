#include "macd_vwapBacktester_plot.hpp"
#include <algorithm>

void plot_MACD_VWAPBacktester (
    const std::vector<Tick>& tickDataVector,
    const std::vector<Trade>& tradeVector,
    int currentFrame) {
    if (tickDataVector.empty() || tradeVector.empty())
        return;

    std::vector<double> buyX, buyY;
    std::vector<double> sellX, sellY;

    for (const auto& trade : tradeVector) {
        auto it = std::find_if(tickDataVector.begin(), tickDataVector.end(),
                               [&](const Tick& t){ return t.date == trade.execution_date; });
        if (it != tickDataVector.end()) {
            size_t index = std::distance(tickDataVector.begin(), it);
            if (index < tickDataVector.size()) {  
                if (trade.order_type == "BUY") {
                    buyX.push_back(static_cast<double>(index + 1));
                    buyY.push_back(trade.strike_price);
                } else if (trade.order_type == "SELL") {
                    sellX.push_back(static_cast<double>(index + 1));
                    sellY.push_back(trade.strike_price);
                }
            }
        }
    }

    // Plot Buy markers (blue triangles)
    if (!buyX.empty()) {
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Up, 8, ImVec4(0, 0, 1, 1)); // blue
        ImPlot::PlotScatter("Buy", buyX.data(), buyY.data(), static_cast<int>(buyX.size()));
    }

    // Plot Sell markers (pink triangles)
    if (!sellX.empty()) {
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Down, 8, ImVec4(1, 0, 1, 1)); // pink
        ImPlot::PlotScatter("Sell", sellX.data(), sellY.data(), static_cast<int>(sellX.size()));
    }
}
