#pragma once
#include <vector>
#include <implot.h>
#include "core/tick.hpp"
#include "core/tech_indicators/macd.hpp"

void plotMACD(const std::vector<Tick>& tickData, int& fastEMA_period, int& slowEMA_period, int& signal_period, int currentFrame, int lookBack);