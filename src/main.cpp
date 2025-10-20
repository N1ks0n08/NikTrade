#include <fmt/core.h>
#include <vector>
#include <fstream>

// core modules
#include "core/tick.hpp"
#include "core/data_loader.hpp"

// UI modules
#include "ui/core/init.hpp"
#include "ui/windows/main_window.hpp"
#include "ui/windows/dataDisplay_window.hpp"

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

    // --------------------- Main loop ---------------------
    while (!glfwWindowShouldClose(window)) {
        startImGuiFrame(window);

        // Get current window size
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // --------- Actual ImGui Widget Layouts -------------
        dataDisplayWindow(window, width, height, tickDataVector);
        mainWindow(window, width, height);

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

