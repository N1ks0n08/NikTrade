#define GLFW_INCLUDE_NONE
#include <iostream>
#include <fmt/core.h>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void mainWindow(GLFWwindow* window, int windowWidth, int windowHeight) {
    // Configure the window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f);       // Round corners
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);      // No borders
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.12f, 1.0f)); // Slightly darker background
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));
    ImGui::Begin("mainWindow", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoMove);

    // Get positions and sizes
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();
    
    // TITLE BAR LOGIC
    {
        // Decorative bar dimensions
        float barHeight = 30.0f;
        ImVec4 barColor = ImVec4(0.18f, 0.55f, 0.90f, 1.0f);
        ImU32 barCol = ImGui::ColorConvertFloat4ToU32(barColor);
        
        // Draw custom decorative bar (top area)
        draw_list->AddRectFilled(
            win_pos,
            ImVec2(win_pos.x + win_size.x, win_pos.y + barHeight),
            barCol
        );

        // Optional: title text on the bar
        ImGui::SetCursorScreenPos(ImVec2(win_pos.x + 15, win_pos.y + 6));
        ImGui::TextUnformatted("NikTrade Dashboard");

        // Offset content below the bar
        ImGui::SetCursorScreenPos(ImVec2(win_pos.x + ImGui::GetStyle().WindowPadding.x,
                                        win_pos.y + barHeight + ImGui::GetStyle().WindowPadding.y));
    }

    // Example content
    ImGui::Text("Window content here...");
    ImGui::Dummy(ImVec2(0.0f, 150.0f));
    ImGui::Text("Rounded corners + custom bar ✅");

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

int main() {
    /* Initialize the library: (REQUIRED BEFORE GLFW FUNCTIONS CAN BE USED)*/
    if (!glfwInit()) {
        fmt::print("Error: GLFW initialization failed...");
        return -1;
    }

    /*Create a windowed mode window and its OpenGL context*/
    /* Make it frameless for a more sleek appearance*/
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow((int)(1000 * main_scale), (int)(750 * main_scale), "NikTrade", nullptr, nullptr);

    /* Check if window was successfully created or no */
    if (!window) {
        glfwTerminate();
        fmt::print("Error: something happened!");
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync (cap frame update speed to monitor refresh rate)
    gladLoadGL(); /* This allows for the actual glClear and glColorClear functions from GL3.3+
                    to work and not crash the window from being drawn 
                    VERY CRITICAL BEFORE STARTING WITH IMGUI INTIALIZATION!!!*/

    /*Initialization of ImGui*/
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable multi-viewport (multiple OS-level windows)


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();



    /*Loop until the user closes the GLFW window*/
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        // Get the glfw window width and height:
        int glfwWindowHWidth, glfwWindowHeight;
        glfwGetWindowSize(window, &glfwWindowHWidth, &glfwWindowHeight);
        ImGui::NewFrame();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 20.0f;
        style.Colors[ImGuiCol_WindowBg].w = 0.0f;  // fully transparent background
        
        // ---------Actual ImGui Widget Layouts -------------
        mainWindow(window, glfwWindowHWidth, glfwWindowHeight);

        // ---------- Rendering ----------
        // Important: use framebuffer size for viewport (fixes HiDPI / flicker problems)
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        // Clear with transparent alpha
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // transparent clear color
        glClear(GL_COLOR_BUFFER_BIT);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }


    /* Termination process */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    fmt::print("Window has been terminated.\n");
    return 0;
}