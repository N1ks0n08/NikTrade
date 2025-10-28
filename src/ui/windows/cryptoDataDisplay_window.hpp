#pragma once
#include <GLFW/glfw3.h>
#include <deque>
#include <boost/lockfree/spsc_queue.hpp>
#include "../../core/flatbuffers/Binance/binance_bookticker_generated.h"

// Reads messages from a deque instead of directly from the concurrent queue.
void cryptoDataDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight,
                             const std::deque<const Binance::BookTicker*>& latestCryptoMessages);