#include <fmt/core.h>
#include <vector>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

// core modules
#include "core/tick.hpp"
#include "core/data_loader.hpp"
#include "core/net/zmq_subscriber.hpp"
#include "core/net/python_launcher.hpp"

// UI modules
#include "ui/core/init.hpp"
#include "ui/windows/dataDisplay_window.hpp"
#include "ui/windows/cryptoDataDisplay_window.hpp"
#include "ui/windows/banner_window.hpp"

// Data communication
#include <boost/lockfree/spsc_queue.hpp>
#include "../src/core/flatbuffers/Binance/binance_bookticker_generated.h"
// ----------- USED FOR getExecutableDir() ---------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>          // includes minwindef.h internally
// ----------- USED FOR getExecutableDir() ---------------
#include <imgui_internal.h>

using json = nlohmann::json;

namespace fs = std::filesystem;

// Helper to get executable directory
fs::path getExecutableDir() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return fs::path(buffer).parent_path();
#else
    char buffer[1024];
    ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (count == -1) return fs::current_path();
    return fs::path(std::string(buffer, count)).parent_path();
#endif
}

int main() {
    // Get the executable directory
    fs::path exeDir = getExecutableDir();
    // --------------------- Initialize window & UI ---------------------
    GLFWwindow* window = initWindow(1000, 750, "NikTrade", exeDir);
    if (!window) return -1;

    // --------------------- Launch Python Binance publisher ---------------------
    fs::path pythonScript = exeDir / "python" / "main.py";

    if (!fs::exists(pythonScript)) {
        fmt::print("Error: Python publisher not found at: {}\n", pythonScript.string());
    } else {
        fmt::print("Launching Python publisher: {}\n", pythonScript.string());

        // Explicit Python executable (matches my PowerShell environment)
        std::string pythonExec = "py -3.13"; 

        // Keep PythonLauncher alive for the duration of main

        static NikTrade::PythonLauncher pythonLauncher(
            pythonScript.string(),
            std::vector<std::string>{}, 
            "C:\\Users\\n1ksn\\AppData\\Local\\Programs\\Python\\Python313\\python.exe"
        );
        pythonLauncher.start();

        // Give it some time to bind sockets
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // --------------------- Load JSON market data ---------------------
    fs::path jsonFile = exeDir / "resources" / "SPY_2025.json";
    std::ifstream file(jsonFile);
    if (!file.is_open()) {
        fmt::print("Error: failed to open file at {}\n", jsonFile.string());
        return -1;
    }

    json jsonData;
    file >> jsonData;
    std::vector<Tick> tickDataVector = json_to_tickDataVector(jsonData);

    // --------------------- Setup ZMQ subscriber ---------------------
    Binance::ZMQSubscriber subscriber(1024); // Queue capacity 1024
    subscriber.start(); // Start the subscriber thread

    // --------------------- Message storage ---------------------
    std::deque<std::vector<uint8_t>> latestCryptoMessages;

    // --------------------- Main loop ---------------------
    while (!glfwWindowShouldClose(window)) {
        startImGuiFrame(window);
        // Before creating DockSpace
        float bannerHeight = 45.0f;
        ImGuiIO io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, bannerHeight));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - bannerHeight));
        
        // --------------------- Create main docking space ---------------------
        ImGuiWindowFlags dockspaceFlags = ImGuiWindowFlags_NoTitleBar | 
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus |
                                        ImGuiWindowFlags_NoBackground;

        ImGui::Begin("DockSpace_Window", nullptr, dockspaceFlags);

        // Create the DockSpace
        ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::End();

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // --------------------- Handle crypto messages from queue ---------------------
        std::vector<uint8_t> msg;
        while (subscriber.pop(msg)) {
            latestCryptoMessages.emplace_back(std::move(msg));

            // Keep only last 50 messages
            if (latestCryptoMessages.size() > 50) {
                latestCryptoMessages.pop_front();
            }
        }
        // --------------------- Render Banner ---------------------
        // Provide current status values
        bool binanceConnected = true; // or track your real connection status
        bool zmqActive = true;        // or track subscriber.isRunning() etc.
        float latencyMs = 12.5f;      // compute your actual latency if needed

        NikTrade::bannerWindow(binanceConnected, zmqActive, latencyMs);

        // --------------------- Render windows ---------------------
    
        dataDisplayWindow(window, width, height, tickDataVector);
        cryptoDataDisplayWindow(window, width, height, latestCryptoMessages);
        
        // --------------------- OpenGL render ---------------------
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        // Visible opaque background
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // --------------------- First-frame dock split ---------------------
        static bool firstFrame = true;
        if (firstFrame) {
            firstFrame = false;

            ImGui::DockBuilderRemoveNode(dockspaceID); // clear old layout
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);
            ImGuiID dock_id_left, dock_id_right;
            ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, &dock_id_right, &dock_id_left);
            ImGui::DockBuilderDockWindow("Equity Data Display", dock_id_left);
            ImGui::DockBuilderDockWindow("Crypto Data Display", dock_id_right);
            ImGui::DockBuilderFinish(dockspaceID);
        }

        endImGuiFrame();
        glfwSwapBuffers(window);
    }

    // --------------------- Cleanup ---------------------
    subscriber.stop();

    shutdownUI(window);

    fmt::print("Window has been terminated.\n");
    return 0;
}

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}
#endif
