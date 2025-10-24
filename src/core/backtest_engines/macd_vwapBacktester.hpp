#pragma once
#include "core/tech_indicators/macd.hpp"
#include "core/tech_indicators/vwap.hpp"
#include "core/backtest_engines/Trade.hpp"

std::vector<Trade> MACD_VWAPBacktestResultCalc (int& fastEMAPeriod, int& slowEMAPeriod, int& signalPeriod
    , double starting_capital, std::vector<Tick>& tickerData);