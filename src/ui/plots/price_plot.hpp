#pragma once
#include <vector>
#include <implot.h>
#include "core/tick.hpp" // defines Tick

void plotPrice(const std::vector<Tick>& tickDataVector, int currentFrame);
