#pragma once
#include <vector>
#include <implot.h>
#include "core/backtest_engines/Trade.hpp"
#include "core/tick.hpp"

// Plots SMA Crossover trades as buy/sell markers on a price chart
void plotSMACrossoverTrades(
    const std::vector<Tick>& tickDataVector,
    const std::vector<Trade>& tradeVector,
    int currentFrame
);
