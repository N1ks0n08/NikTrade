#include <vector>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <deque>
#include <fmt/core.h>

// util modules
#include "utils/file_logger.hpp"

// core modules
#include "core/tick.hpp"
#include "core/binance_kline.hpp"
#include "core/data_loader.hpp"
#include "core/net/zmq_subscriber.hpp"
#include "core/net/python_launcher.hpp"
#include "core/net/zmq_control_client.hpp"

// UI modules
#include "ui/core/init.hpp"
#include "ui/windows/dataDisplay_window.hpp"
#include "ui/windows/cryptoDataDisplay_window.hpp"
#include "ui/windows/cryptoChartDisplay_window.hpp"
#include "ui/windows/banner_window.hpp"

// Data communication
#include <boost/lockfree/spsc_queue.hpp>
#include "../src/core/flatbuffers/Binance/binance_bookticker_generated.h"
#include "../src/core/flatbuffers/Binance/binance_kline_generated.h"

// ----------- USED FOR getExecutableDir() ---------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imgui_internal.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

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

void runHiddenCommand(const std::wstring& cmd)
{
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    // 👇 Hide window
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // CreateProcess needs a writable buffer
    std::wstring mutableCmd = cmd;

    BOOL success = CreateProcessW(
        nullptr,
        mutableCmd.data(),  // command line
        nullptr, nullptr,   // process/thread security
        FALSE,              // inherit handles
        CREATE_NO_WINDOW,   // absolutely no visible window
        nullptr, nullptr,   // environment, cwd
        &si, &pi
    );

    if (success) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void forceClosePorts(FileLogger& logger) {
#ifdef _WIN32
    logger.logInfo("[INFO] Forcibly closing any lingering ZMQ ports (hidden) ...");

    runHiddenCommand(
        L"powershell -Command \""
        L"$ports = @(5555, 5556, 5560); "
        L"$ports | ForEach-Object { "
        L"Get-NetTCPConnection -LocalPort $_ -ErrorAction SilentlyContinue | "
        L"ForEach-Object { "
        L"try { Stop-Process -Id $_.OwningProcess -Force -ErrorAction SilentlyContinue } catch {} "
        L"} }\""
    );
#else
    logger.logInfo("[INFO] Forcibly closing any lingering ZMQ ports on Unix...");

    system("lsof -i :5555 -t | xargs -r kill -9");
    system("lsof -i :5556 -t | xargs -r kill -9");
    system("lsof -i :5560 -t | xargs -r kill -9");
#endif
}

int main() {
    // --------------------- Initialize Logger ---------------------
    FileLogger logger("NikTrade.log");
    logger.logInfo("Starting NikTrade application...");
    forceClosePorts(logger);

    // --------------------- Initialize window & UI ---------------------
    fs::path exeDir = getExecutableDir();
    GLFWwindow* window = initWindow(1000, 750, "NikTrade", exeDir);
    if (!window) {
        logger.logInfo("Failed to initialize window.");
        return -1;
    }
    logger.logInfo("Window initialized successfully.");

    std::unique_ptr<NikTrade::PythonLauncher> pythonLauncher;
    // --------------------- Launch Python Binance publisher ---------------------
    fs::path pythonScript = exeDir / "python" / "main.py";
    if (!fs::exists(pythonScript)) {
        logger.logInfo(fmt::format("Python publisher not found at: {}", pythonScript.string()));
    } else {
        logger.logInfo(fmt::format("Launching Python publisher: {}", pythonScript.string()));

        pythonLauncher = std::make_unique<NikTrade::PythonLauncher>(
            pythonScript.string(),
            std::vector<std::string>{}, 
            "C:\\Users\\n1ksn\\AppData\\Local\\Programs\\Python\\Python313\\python.exe"
        );
        pythonLauncher->start();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        logger.logInfo("Python publisher started.");
    }

    // --------------------- Load JSON market data ---------------------
    fs::path jsonFile = exeDir / "resources" / "SPY_2025.json";
    std::ifstream file(jsonFile);
    if (!file.is_open()) {
        logger.logInfo(fmt::format("Failed to open JSON file at {}", jsonFile.string()));
        return -1;
    }

    json jsonData;
    file >> jsonData;
    std::vector<Tick> tickDataVector = json_to_tickDataVector(jsonData);
    logger.logInfo(fmt::format("Loaded {} ticks from JSON.", tickDataVector.size()));

    // --------------------- Setup ZMQ subscribers ---------------------
    Binance::ZMQSubscriber bookticker_sub(1024, "tcp://127.0.0.1:5555");
    bookticker_sub.start();
    Binance::ZMQSubscriber kline_sub(1024, "tcp://127.0.0.1:5556");
    kline_sub.start();
    logger.logInfo("ZMQ subscribers started for BookTicker and Kline streams.");

    // --------------------- Storage ---------------------
    std::deque<std::vector<uint8_t>> latestCryptoMessages;
    std::deque<std::vector<uint8_t>> latestKlineMessages;
    std::deque<KlineData> klineDeque;

    // --------------------- Flags & Timers ---------------------
    static auto lastKlineRequest = std::chrono::steady_clock::now();
    static const std::chrono::seconds requestInterval(5);

    // --------------------- Main loop ---------------------
    while (!glfwWindowShouldClose(window)) {
        startImGuiFrame(window);

        float bannerHeight = 45.0f;
        ImGuiIO io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, bannerHeight));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - bannerHeight));

        ImGuiWindowFlags dockspaceFlags = ImGuiWindowFlags_NoTitleBar |
                                          ImGuiWindowFlags_NoCollapse |
                                          ImGuiWindowFlags_NoResize |
                                          ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoBringToFrontOnFocus |
                                          ImGuiWindowFlags_NoBackground;

        ImGui::Begin("DockSpace_Window", nullptr, dockspaceFlags);
        ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // --------------------- Handle crypto messages ---------------------
        std::vector<uint8_t> msg;
        while (bookticker_sub.pop(msg)) {
            latestCryptoMessages.emplace_back(std::move(msg));
            if (latestCryptoMessages.size() > 50) latestCryptoMessages.pop_front();
        }

        // --------------------- Periodic Historical Klines Request ---------------------
        auto now = std::chrono::steady_clock::now();
        if (now - lastKlineRequest >= requestInterval) {
            ZMQControlClient controlClient("tcp://127.0.0.1:5560");
            if (controlClient.requestHistoricalKlines("fire_klines BTCUSDT")) {
                logger.logInfo("[INFO] Periodic request for historical candles sent");
            } else {
                logger.logInfo("[WARN] Failed to request historical candles");
            }
            lastKlineRequest = now;
        }

        // --------------------- Read Kline messages continuously ---------------------
        std::vector<uint8_t> kline_msg;
        while (kline_sub.pop(kline_msg)) {
            const Binance::Klines* fb_klines = Binance::GetKlines(kline_msg.data());
            if (!fb_klines || !fb_klines->klines()) continue;

            for (auto kl : *(fb_klines->klines())) {
                KlineData k;
                k.open_time  = kl->open_time();
                k.open       = std::stod(kl->open_price()->str());
                k.high       = std::stod(kl->high_price()->str());
                k.low        = std::stod(kl->low_price()->str());
                k.close      = std::stod(kl->close_price()->str());
                k.volume     = std::stod(kl->volume()->str());
                k.close_time = kl->close_time();
                klineDeque.push_back(k);
            }

            while (klineDeque.size() > 500) klineDeque.pop_front();

            logger.logInfo(fmt::format("[DEBUG] Added {} Klines. Deque size: {}", fb_klines->klines()->size(), klineDeque.size()));
        }

        // --------------------- Render Banner & Windows ---------------------
        bool binanceConnected = true;
        bool zmqActive        = true;
        float latencyMs       = 12.5f;
        NikTrade::bannerWindow(binanceConnected, zmqActive, latencyMs);

        dataDisplayWindow(window, width, height, tickDataVector);
        cryptoDataDisplayWindow(window, width, height, latestCryptoMessages);
        cryptoChartDisplayWindow(window, width, height, klineDeque);

        // --------------------- OpenGL Render ---------------------
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // --------------------- Dock setup first frame ---------------------
        static bool firstFrame = true;
        if (firstFrame) {
            firstFrame = false;
            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);

            ImGuiID dock_id_left, dock_id_right;
            ImGuiID dock_id_top, dock_id_bottom;

            ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.33f, &dock_id_right, &dock_id_left);
            ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, &dock_id_bottom, &dock_id_top);

            // Swap these two lines to change vertical order
            ImGui::DockBuilderDockWindow("Crypto Chart Display", dock_id_top);
            ImGui::DockBuilderDockWindow("Equity Data Display", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Crypto Data Display", dock_id_right);
            // NOTE: THE NAMES MUST MATCH THE WINDOW NAMES IN THEIR RESPECTIVE UI CODE
            ImGui::DockBuilderFinish(dockspaceID);
        }

        endImGuiFrame();
        glfwSwapBuffers(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // --------------------- Cleanup ---------------------
    bookticker_sub.stop();
    kline_sub.stop();
    pythonLauncher->stop();

    forceClosePorts(logger);

    shutdownUI(window);

    logger.logInfo("Window has been terminated. Exiting application.");
    return 0;
}

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}
#endif
