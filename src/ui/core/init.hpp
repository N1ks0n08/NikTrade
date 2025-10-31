#pragma once
#include <GLFW/glfw3.h>
#include <filesystem>
#include <imgui.h>

/* Initializes GLFW, OpenGL, and ImGui context, returns a pointer to the created GLFW window */
GLFWwindow* initWindow(int width, int height, const char* title, std::filesystem::path);

/* Begins a new ImGui frame for rendering widgets */
void startImGuiFrame(GLFWwindow* window);

/* Ends the ImGui frame and renders all draw data */
void endImGuiFrame();

/* Shutdown ImGui and GLFW, terminate window context */
void shutdownUI(GLFWwindow* window);
