#pragma once
#include <GLFW/glfw3.h>
#include <deque>
#include "../../core/flatbuffers/Binance/binance_bookticker_generated.h"

// Changed to accept deque of vectors by const reference
void cryptoDataDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight,
                            const std::deque<std::vector<uint8_t>>& latestCryptoMessages);