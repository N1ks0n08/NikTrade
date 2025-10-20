#pragma once
#include <vector>
#include <core/tick.hpp>

std::vector<double> emaCalc(int ema_interval, const std::vector<Tick>& ticker_data);
std::vector<double> emaCalc(int ema_interval, const std::vector<double>& initial_vector);