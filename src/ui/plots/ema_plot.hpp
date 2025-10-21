#pragma once
#include <vector>
#include <implot.h>
#include "core/tick.hpp"
#include "core/tech_indicators/ema.hpp"

void plotEMA(const std::vector<Tick>& tickData, int period, int currentFrame, const char* label, ImVec4 color);
