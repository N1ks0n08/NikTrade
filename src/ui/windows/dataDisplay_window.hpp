#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include "core/tick.hpp"

void dataDisplayWindow(GLFWwindow* window, int windowWidth, int windowHeight, std::vector<Tick>& tickDataVector);