#pragma once
#include <vector>
#include <implot.h>
#include "core/tick.hpp"
#include "core/tech_indicators/rsi.hpp"

void plotRSI(const std::vector<Tick>& tickData, int& rsi_interval, int& currentFrame, int& lookBack);