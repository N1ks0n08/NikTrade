#include <vector>
#include <deque>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <fmt/core.h>

// Utils & Logging
#include "utils/file_logger.hpp"

// Core modules
#include "core/tick.hpp"
#include "core/binance_kline.hpp"
#include "core/data_loader.hpp"
#include "core/window_state.hpp"
#include "core/BBO.hpp"
#include "core/BinanceBookTickerDecoder.hpp"
#include "core/SymbolRequest.hpp"

// Networking
#include "core/net/zmq_subscriber.hpp"
#include "core/net/python_launcher.hpp"
#include "core/net/zmq_control_client.hpp"

// UI
#include "ui/core/init.hpp"
#include "ui/windows/dataDisplay_window.hpp"
#include "ui/windows/orderBookDisplay_window.hpp"
#include "ui/windows/chartDisplay_window.hpp"
#include "ui/windows/banner_window.hpp"

// FlatBuffers
#include "../src/core/flatbuffers/Binance/binance_bookticker_generated.h"
#include "../src/core/flatbuffers/Binance/binance_kline_generated.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imgui_internal.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

// --------------------------- Helpers ---------------------------
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

void forceClosePorts(FileLogger& logger) {
#ifdef _WIN32
    logger.logInfo("[INFO] Forcibly closing lingering ZMQ ports ...");
    std::wstring cmd = L"powershell -Command \"$ports = @(5555, 5556, 5560, 5561); "
                       L"$ports | ForEach-Object { Get-NetTCPConnection -LocalPort $_ -ErrorAction SilentlyContinue | "
                       L"ForEach-Object { try { Stop-Process -Id $_.OwningProcess -Force -ErrorAction SilentlyContinue } catch {} } }\"";
    STARTUPINFOW si{}; PROCESS_INFORMATION pi{}; si.cb = sizeof(si); si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    std::wstring mutableCmd = cmd;
    if (CreateProcessW(nullptr, mutableCmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    }
#else
    system("lsof -i :5555 -t | xargs -r kill -9");
    system("lsof -i :5556 -t | xargs -r kill -9");
    system("lsof -i :5560 -t | xargs -r kill -9");
    system("lsof -i :5561 -t | xargs -r kill -9");
#endif
}

// --------------------------- Main ---------------------------
int main() {
    // ------------------ Logger ------------------
    // Delete old log BEFORE creating the logger
    if (fs::exists("NikTrade.log")) {
        try {
            fs::remove("NikTrade.log");
        } catch (const std::exception& e) {
            // Use logger-free thread-safe fallback: Write to Windows Debug output
    #ifdef _WIN32
            OutputDebugStringA(("Could not remove old log: " + std::string(e.what()) + "\n").c_str());
    #else
            std::cerr << "[WARN] Could not remove old log: " << e.what() << std::endl;
    #endif
        }
    }

    FileLogger logger("NikTrade.log");
    logger.logInfo("Starting NikTrade...");

    forceClosePorts(logger);

    // ------------------ Window/UI ------------------
    fs::path exeDir = getExecutableDir();
    GLFWwindow* window = initWindow(1000, 750, "NikTrade", exeDir);
    if (!window) { logger.logInfo("Failed to initialize window."); return -1; }
    logger.logInfo("Window initialized successfully.");

    // ------------------ Python Publisher ------------------
    fs::path pythonScript = exeDir / "python" / "main.py";
    std::unique_ptr<NikTrade::PythonLauncher> pythonLauncher;
    if (!fs::exists(pythonScript)) {
        logger.logInfo(fmt::format("Python publisher not found at: {}", pythonScript.string()));
    } else {
        pythonLauncher = std::make_unique<NikTrade::PythonLauncher>(
            pythonScript.string(),
            std::vector<std::string>{},
            "python"
        );
        pythonLauncher->start();
        std::this_thread::sleep_for(std::chrono::seconds(2)); // allow Python to initialize
        logger.logInfo("Python publisher started.");
    }

    // ------------------ Load Symbols ------------------
    std::vector<std::string> symbols;
    symbols.reserve(14000); // Reserve space for symbols from Binance + NASDAQ Basic securities
    fs::path symbolsFile = exeDir / "python" / "binance_symbols.json";
    std::ifstream symbolsStream(symbolsFile);
    if (!symbolsStream.is_open()) {
        logger.logInfo(fmt::format("Failed to open symbols file at {}", symbolsFile.string()));
        return -1;
    }
    json symbolsJson; symbolsStream >> symbolsJson;
    for (const auto& symbol : symbolsJson) symbols.push_back(symbol.get<std::string>());
    logger.logInfo(fmt::format("Loaded {} symbols.", symbols.size()));

    /*
    // ------------------ Load Tick Data ------------------
    fs::path jsonFile = exeDir / "resources" / "SPY_2025.json";
    std::ifstream file(jsonFile);
    if (!file.is_open()) { logger.logInfo(fmt::format("Failed to open JSON file at {}", jsonFile.string())); return -1; }
    json jsonData; file >> jsonData;
    std::vector<Tick> tickDataVector = json_to_tickDataVector(jsonData);
    logger.logInfo(fmt::format("Loaded {} ticks from JSON.", tickDataVector.size()));
    */
    // ------------------ ZMQ Subscribers ------------------
    Binance::ZMQSubscriber bookticker_sub(524288, "tcp://127.0.0.1:5555"); bookticker_sub.start();
    Binance::ZMQSubscriber kline_sub(262144, "tcp://127.0.0.1:5556"); kline_sub.start();
    Binance::ZMQSubscriber latency_sub(1024, "tcp://127.0.0.1:5561"); latency_sub.start();
    logger.logInfo("ZMQ subscribers started.");

    ZMQControlClient controlClient("tcp://127.0.0.1:5560");
    logger.logInfo("ZMQ Control Client connected.");

    // ------------------ Storage ------------------
    //std::vector<int> activeWindowIDs; // IDs of currently active windows
    //activeWindowIDs.reserve(50); // Reserve space for windowIDs (Max of 100 symbols)
    //activeWindowIDs.emplace_back(0); // Start with window ID 0 active

    std::vector<SymbolRequest> pendingRRequests; // Symbols requested by windows
    pendingRRequests.reserve(10); // Reserve space for symbol requests from windows per main loop iteration

    // THIS IS ACCESSED BY WINDOW ID
    std::vector<WindowBBO> activeBBOWindows; // Symbols currently being displayed in windows
    activeBBOWindows.reserve(50); // Reserve space for symbols being displayed (Max of 100 symbols)
    activeBBOWindows.emplace_back(WindowBBO{true, 0, BBO{}}); // Start with window ID 0

    std::vector<uint8_t> latestFlatbufferMessage;

    std::deque<std::vector<uint8_t>> latestKlineMessages;
    std::deque<KlineData> klineDeque;

    std::string latestLatencyMessage = "Latency: Loading...";

    std::deque<std::string> currentSymbol;
    if (!symbols.empty()) currentSymbol.push_back(symbols[0]);

    static auto lastKlineRequest = std::chrono::steady_clock::now();
    static const std::chrono::seconds requestInterval(5);

    // ------------------ Main Loop ------------------
    while (!glfwWindowShouldClose(window)) {
        // Execute any pending symbol requests from windows
        if (!pendingRRequests.empty()) {
            for (const SymbolRequest& req : pendingRRequests) {
                std::string reply;
                bool ok = controlClient.sendControlRequest(
                    fmt::format("start_symbol {}", req.requestedSymbol),
                    reply,
                    logger,
                    500
                );

                //BBO bbo = decodeToBBO(latestFlatbufferMessage, logger);
                // handle activeWindows state
                if (ok) {
                    logger.logInfo(fmt::format("[INFO] Start symbol: {}", reply));
                    activeBBOWindows[req.windowID] = WindowBBO{true, req.windowID, decodeToBBO(latestFlatbufferMessage, logger)}; // verbose/unnecessary
                }
                else {
                    logger.logInfo("[WARN] Failed to switch symbol.");
                    activeBBOWindows[req.windowID].currentBBO.error = "Failed to start symbol stream.";
                }
            }
            // All processed, clear pending requests
            pendingRRequests.clear();
        }

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
        ImGui::DockSpace(dockspaceID, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        int width, height; glfwGetWindowSize(window, &width, &height);

        // ------------------ Handle BookTicker ------------------
        std::vector<uint8_t> msg;
        if (bookticker_sub.pop(msg)) {
            latestFlatbufferMessage = std::move(msg);
            logger.logInfo("BookTicker msg received, size: " + std::to_string(latestFlatbufferMessage.size()));
        }

        // ------------------ Decode latest BBO for all windows ------------------
        for (auto& win : activeBBOWindows) {
            win.currentBBO = decodeToBBO(latestFlatbufferMessage, logger);
        }

        // ------------------ Handle Latency ------------------
        std::vector<uint8_t> latency_msg;
        while (latency_sub.pop(latency_msg)) {
            std::string str_msg(latency_msg.begin(), latency_msg.end());
            auto space_pos = str_msg.find(' ');
            if (space_pos != std::string::npos) latestLatencyMessage = str_msg.substr(space_pos + 1);
        }

        // ------------------ Periodic Historical Klines ------------------
        auto now = std::chrono::steady_clock::now();
        if (!currentSymbol.empty() && now - lastKlineRequest >= requestInterval) {
            lastKlineRequest = now;
            std::string reply;
            bool ok = controlClient.sendControlRequest(fmt::format("fire_klines {}", currentSymbol[0]), reply, logger, 1000);
            if (!ok) logger.logInfo("[WARN] Historical klines request failed: " + reply);
        }

        // ------------------ Read Kline Messages ------------------
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
        }

        // ------------------ Render UI ------------------
        bool binanceConnected = true;
        bool zmqActive = true;
        NikTrade::bannerWindow(binanceConnected, zmqActive, latestLatencyMessage);
        // dataDisplayWindow(window, width, height, tickDataVector); // TESTING PURPOSES
        for (auto& win : activeBBOWindows) {
            if (!win.active) continue;
            orderBookDisplayWindow(window, width, height, symbols, logger, pendingRRequests, activeBBOWindows, win.windowID);
        }
        chartDisplayWindow(window, width, height, klineDeque);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        static bool firstFrame = true;
        if (firstFrame) {
            firstFrame = false;
            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);

            ImGuiID dock_id_left, dock_id_right, dock_id_top, dock_id_bottom;
            ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.33f, &dock_id_right, &dock_id_left);
            ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, &dock_id_bottom, &dock_id_top);

            ImGui::DockBuilderDockWindow("Chart Display", dock_id_top);
            // ImGui::DockBuilderDockWindow("Equity Data Display", dock_id_bottom); // TESTING PURPOSES
            ImGui::DockBuilderDockWindow("Orderbook Display", dock_id_right);
            ImGui::DockBuilderFinish(dockspaceID);
        }

        endImGuiFrame();
        glfwSwapBuffers(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // ------------------ Cleanup ------------------
    bookticker_sub.stop();
    kline_sub.stop();
    latency_sub.stop();
    if (pythonLauncher) pythonLauncher->stop();
    forceClosePorts(logger);
    shutdownUI(window);

    logger.logInfo("Application terminated cleanly.");
    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return main(); }
#endif
