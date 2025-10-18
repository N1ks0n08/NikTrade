#pragma once
#include <vector>
#include "core/tick.hpp"

std::vector<double> rsiCalc (int rsi_interval, const std::vector<Tick>& ticker_data);