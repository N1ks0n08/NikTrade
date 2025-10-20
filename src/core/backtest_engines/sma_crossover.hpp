#pragma once
#include "core/tech_indicators/sma.hpp"
#include "Trade.hpp"

std::vector<Trade> sma_crossover_result(int& fastSMAPeriod, int& slowSMAPeriod, double& startingCapital, std::vector<Tick>& ticker_data);
