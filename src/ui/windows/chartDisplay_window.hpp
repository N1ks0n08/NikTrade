#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <core/binance_kline.hpp>
#include <core/flatbuffers/Binance/binance_kline_generated.h>
#include <deque>

void chartDisplayWindow(GLFWwindow* window,
                              int windowWidth,
                              int windowHeight,
                              const std::deque<KlineData>& klines);