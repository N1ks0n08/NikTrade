#include <fmt/core.h>
#include <glad/glad.h> // GLAD COMES FIRST BEFORE GLFW AND WHATNOT
#include "init.hpp" // CONTAINS GLFW
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
namespace fs = std::filesystem;

GLFWwindow* initWindow(int width, int height, const char* title, fs::path exeDir) {
    /* Initialize the library */
    if (!glfwInit()) { fmt::print("Error: GLFW initialization failed..."); return nullptr; }

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) { glfwTerminate(); return nullptr; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    gladLoadGL(); 

    /* ImGui Initialization */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    fs::path iniPath = exeDir / "imgui_layout.ini";
    io.IniFilename = nullptr; 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 

    // Set font size to 30
    float fontSize = 30.0f;
#ifdef _WIN32
    const char* fontPath = "C:/Windows/Fonts/Arial.ttf";
#else
    const char* fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
    if (fs::exists(fontPath)) {
        io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath, fontSize);
    } else {
        io.FontDefault = io.Fonts->AddFontDefault(); // fallback
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Adjust style for readability
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 15.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    return window;
}

void startImGuiFrame(GLFWwindow* window) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = 0.0f; // fully transparent background
}

void endImGuiFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Optional: update ImGui viewports if enabled
    // if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     ImGui::UpdatePlatformWindows();
    //     ImGui::RenderPlatformWindowsDefault();
    // }
}

void shutdownUI(GLFWwindow* window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
