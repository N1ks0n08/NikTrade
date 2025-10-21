#pragma once
#include <vector>
#include <implot.h>
#include "core/tick.hpp"  // Tick
#include "core/tech_indicators/sma.hpp"

// Plots a single SMA line with proper offset
void plotSMA(const std::vector<Tick>& tickData, int period, int currentFrame, const char* label, ImVec4 color);
