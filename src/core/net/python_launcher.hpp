#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <thread>

namespace NikTrade {

class PythonLauncher {
public:
    // Constructor: specify Python script, optional arguments, optional Python executable
    PythonLauncher(const std::string& scriptPath,
                   const std::vector<std::string>& args = {},
                   const std::string& pythonExecutable = "");

    ~PythonLauncher();

    void start();   // Launch Python asynchronously
    void stop();    // Terminate Python process
    bool isRunning() const;

private:
    std::string scriptPath_;
    std::vector<std::string> args_;
    std::string pythonPath_;             // Path to Python executable
    std::atomic<bool> running_{false};   // true if Python is running
    std::thread workerThread_;

#ifdef _WIN32
    void startWindows();
    void stopWindows();
    void* processHandle_ = nullptr;      // HANDLE for Windows process
#else
    void startUnix();
    void stopUnix();
    pid_t childPid_ = -1;                // PID for Unix process
#endif
};

} // namespace NikTrade
