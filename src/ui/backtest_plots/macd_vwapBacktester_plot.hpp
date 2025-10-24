#pragma once
#include <vector>
#include <implot.h>
#include "core/backtest_engines/Trade.hpp"
#include "core/tick.hpp"

// Plots MACD + VWAP Backtester trades as buy/sell markers on a price chart
void plot_MACD_VWAPBacktester (
    const std::vector<Tick>& tickDataVector,
    const std::vector<Trade>& tradeVector,
    int currentFrame
);
