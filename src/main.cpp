#include <fmt/core.h>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>      // for std::system
#include <filesystem>   // for std::filesystem::path

// core modules
#include "core/tick.hpp"
#include "core/data_loader.hpp"
#include "core/net/zmq_subscriber.hpp"             // ZMQ Subscriber with FlatBuffers

// UI modules
#include "ui/core/init.hpp"
#include "ui/windows/main_window.hpp"
#include "ui/windows/dataDisplay_window.hpp"
#include "ui/windows/cryptoDataDisplay_window.hpp"  // ImGui window for crypto

// Data communication
#include <boost/lockfree/spsc_queue.hpp>           // Lock-free queue
#include "../src/core/flatbuffers/Binance/binance_bookticker_generated.h"  // FlatBuffers

using json = nlohmann::json;

/*
SAMPLE JSON DATA FORMAT FROM StockData.org End Of Day DATA API:
{
    "meta": {
        "date_from": "2023-03-18",
        "date_to": "2023-09-14",
        "max_period_days": 180
    },
    "data": [
        {
            "date": "2023-09-12T00:00:00.000Z",
            "open": 179.49,
            "high": 180.11,
            "low": 174.84,
            "close": 176.29,
            "volume": 1454605
        }, ...
    ]
}
*/

/* FOR DEBUGGING PURPOSES
void printData(std::vector<Tick>& tickDataVector) {
    for (int index = 0; index < tickDataVector.size(); index++) {
        fmt::print("The highest SPY price today on {} was: {}\n", tickDataVector[index].date, tickDataVector[index].high);
    }
} */

int main() {
    // --------------------- Initialize window & UI ---------------------
    GLFWwindow* window = initWindow(1000, 750, "NikTrade");
    if (!window) return -1;

    // --------------------- Launch Python Binance publisher ---------------------
        {
            namespace fs = std::filesystem;
            fs::path pythonScript = fs::current_path() / "python" / "main.py";

            // Ensure script exists before launching
            if (!fs::exists(pythonScript)) {
                fmt::print("Error: Python publisher not found at: {}\n", pythonScript.string());
            } else {
                std::string command = "python \"" + pythonScript.string() + "\"";

    #ifdef _WIN32
                // Run silently in the background (no new window)
                command = "start /B " + command;
    #else
                // On Linux/macOS: run detached
                command += " &";
    #endif

                fmt::print("Launching Python publisher: {}\n", command);
                std::system(command.c_str());

                // Give it a moment to start and bind ZMQ socket
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

    // --------------------- Load JSON market data ---------------------
    std::ifstream file("SPY_2025.json");
    if (!file.is_open()) {
        fmt::print("Error: failed to open file!\n");
        return -1;
    }

    // create an empty JSON object of nlohmann::json type
    json jsonData;
    // parse text from file into JSON structured object
    file >> jsonData;
    // get an array of individual tick data per EOD from the selected ticker
    std::vector<Tick> tickDataVector = json_to_tickDataVector(jsonData);

    // --------------------- Setup lock-free queue ---------------------
    // Queue for subscriber (thread-safe)
    auto cryptoQueue = new boost::lockfree::spsc_queue<const Binance::BookTicker*>(1024);

    // Deque for UI display (latest 50 messages)
    std::deque<const Binance::BookTicker*> latestCryptoMessages;

    // --------------------- Setup ZMQ subscriber ---------------------
    ZMQSubscriber subscriber(cryptoQueue, "tcp://127.0.0.1:5555");

    // Start subscriber in a separate thread
    std::thread subscriberThread([&]() {
        subscriber.run();
    });

    // --------------------- Main loop ---------------------
    while (!glfwWindowShouldClose(window)) {
        startImGuiFrame(window);

        // Get current window size
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // --------- Handle crypto messages from queue -------------
        const Binance::BookTicker* msg;
        while (cryptoQueue->pop(msg)) {
            latestCryptoMessages.push_back(msg);
            if (latestCryptoMessages.size() > 50) {  // keep last 50 messages
                latestCryptoMessages.pop_front();
            }
        }

        // --------- Actual ImGui Widget Layouts -------------
        dataDisplayWindow(window, width, height, tickDataVector);
        mainWindow(window, width, height);
        cryptoDataDisplayWindow(window, width, height, latestCryptoMessages); // crypto

        // ---------- Rendering ----------
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        // Clear with transparent alpha
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        endImGuiFrame();
        glfwSwapBuffers(window);
    }

    // --------------------- Cleanup ---------------------
    subscriber.stop();
    if (subscriberThread.joinable()) subscriberThread.join();
    delete cryptoQueue;   // free heap memory

    shutdownUI(window);
    fmt::print("Window has been terminated.\n");

    return 0;
}


// --------------------- Windows GUI subsystem wrapper ---------------------
#ifdef _WIN32
#ifdef WIN32
#include <windows.h>

// This wrapper allows your main() to work even in WIN32 (GUI) subsystem builds
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // MSVC provides __argc and __argv for the console-style arguments
    return main(); // no need to add __argc, __argv for now!
}
#endif
#endif
// -------------------------------------------------------------------------

