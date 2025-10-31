#include <fmt/core.h>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <chrono>

// core modules
#include "core/tick.hpp"
#include "core/data_loader.hpp"
#include "core/net/zmq_subscriber.hpp"

// UI modules
#include "ui/core/init.hpp"
#include "ui/windows/main_window.hpp"
#include "ui/windows/dataDisplay_window.hpp"
#include "ui/windows/cryptoDataDisplay_window.hpp"

// Data communication
#include <boost/lockfree/spsc_queue.hpp>
#include "../src/core/flatbuffers/Binance/binance_bookticker_generated.h"

using json = nlohmann::json;

int main() {
    // --------------------- Initialize window & UI ---------------------
    GLFWwindow* window = initWindow(1000, 750, "NikTrade");
    if (!window) return -1;

    // --------------------- Launch Python Binance publisher ---------------------
    namespace fs = std::filesystem;
    fs::path pythonScript = fs::current_path().parent_path() / "python" / "main.py";

    if (!fs::exists(pythonScript)) {
        fmt::print("Error: Python publisher not found at: {}\n", pythonScript.string());
    } else {
        std::string command = "python \"" + pythonScript.string() + "\"";

    #ifdef _WIN32
        command = "start /B " + command; // run silently in background
    #else
        command += " &";
    #endif

        fmt::print("Launching Python publisher: {}\n", command);
        std::thread([command]() { std::system(command.c_str()); }).detach();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // allow time to bind socket
    }

    // --------------------- Load JSON market data ---------------------
    std::ifstream file("SPY_2025.json");
    if (!file.is_open()) {
        fmt::print("Error: failed to open file!\n");
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

        // --------------------- Render windows ---------------------
        dataDisplayWindow(window, width, height, tickDataVector);
        cryptoDataDisplayWindow(window, width, height, latestCryptoMessages);
        mainWindow(window, width, height);

        // --------------------- OpenGL render ---------------------
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        // Visible opaque background
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        endImGuiFrame();
        glfwSwapBuffers(window);
        //fmt::print("Hello!");
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