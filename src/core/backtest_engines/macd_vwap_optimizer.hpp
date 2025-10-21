#pragma once
#include "core/tech_indicators/macd.hpp"
#include "core/tech_indicators/vwap.hpp"
#include "core/backtest_engines/Trade.hpp"

struct optimizer_result {
    double totalPnL;
    double winRate;
    double averageTradePnL;
    double maxDrawdown;
};

optimizer_result getOptimizerResult(int& fastEMAPeriod, int& slowEMAPeriod, int& signalPeriod, std::vector<Tick>& tickerData);