#pragma once
#include <vector>
#include <core/tick.hpp>

struct MACDResult {
    std::vector<double> macd;
    std::vector<double> signal;
    std::vector<double> histogram;
};

MACDResult macdCalc(int fast_EMA_period, int slow_EMA_period, int signal_period, const std::vector<Tick>& ticker_data);