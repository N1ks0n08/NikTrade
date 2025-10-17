#pragma once
#include <vector>
#include "core/tick.hpp"

std::vector<double> smaCalc(int sma_interval, const std::vector<Tick>& ticker_data);