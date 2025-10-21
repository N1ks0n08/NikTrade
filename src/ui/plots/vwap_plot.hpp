#pragma once
#include <vector>
#include <implot.h>
#include "core/tick.hpp"
#include "core/tech_indicators/vwap.hpp"

void plotVWAP(const std::vector<Tick>& tickData, int currentFrame, const char* label, ImVec4 color);
