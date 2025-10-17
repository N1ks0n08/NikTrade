#include <fmt/core.h>
#include <glad/glad.h> // GLAD COMES FIRST BEFORE GLFW AND WHATNOT
#include "init.hpp" // CONTAINS GLFW
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

GLFWwindow* initWindow(int width, int height, const char* title) {
    /* Initialize the library: (REQUIRED BEFORE GLFW FUNCTIONS CAN BE USED) */
    if (!glfwInit()) { fmt::print("Error: GLFW initialization failed..."); return nullptr; }

    /*Create a windowed mode window and its OpenGL context*/
    /* Make it frameless for a more sleek appearance */
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    /* Check if window was successfully created or no */
    if (!window) { glfwTerminate(); return nullptr; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    gladLoadGL(); /* VERY CRITICAL BEFORE STARTING WITH IMGUI INITIALIZATION!!! */

    /*Initialization of ImGui*/
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext(); // SEPARATE ImPlot CONTEXT REQUIRED TO START

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable multi-viewport

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);  // Install GLFW callbacks
    ImGui_ImplOpenGL3_Init();

    return window;
}

void startImGuiFrame(GLFWwindow* window) {
    /* Poll events */
    glfwPollEvents();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Optional: configure style each frame
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 20.0f;
    style.Colors[ImGuiCol_WindowBg].w = 0.0f;  // fully transparent background
}

void endImGuiFrame() {
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdownUI(GLFWwindow* window) {
    /* Termination process */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext(); // REQUIRED BEFORE ImGui CONTEXT DELETION
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
