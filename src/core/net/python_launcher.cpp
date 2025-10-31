#include "python_launcher.hpp"
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <fmt/core.h>
namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

namespace NikTrade {

PythonLauncher::PythonLauncher(const std::string& scriptPath,
                               const std::vector<std::string>& args,
                               const std::string& pythonExecutable)
    : scriptPath_(scriptPath),
      args_(args),
      pythonPath_(pythonExecutable.empty() ? "python" : pythonExecutable) {}

PythonLauncher::~PythonLauncher() {
    stop();
    if (workerThread_.joinable()) workerThread_.join();
}

bool PythonLauncher::isRunning() const { return running_; }

void PythonLauncher::start() {
    if (running_) return;
    running_ = true;

    workerThread_ = std::thread([this]() {
#ifdef _WIN32
        startWindows();
#else
        startUnix();
#endif
    });
}

void PythonLauncher::stop() {
    if (!running_) return;
    running_ = false;

#ifdef _WIN32
    stopWindows();
#else
    stopUnix();
#endif
}

// ------------------- Windows Implementation -------------------
#ifdef _WIN32
void PythonLauncher::startWindows() {
    // Redirect stdout/stderr to a log file
    fs::path logFile = fs::path(scriptPath_).parent_path() / "python_log.txt";
    std::string workingDir = fs::path(scriptPath_).parent_path().string();

    // Use cmd /C to allow redirection
    std::string command = "cmd /C \"" + pythonPath_ + " \"" + scriptPath_ + "\"";
    for (const auto& arg : args_) command += " \"" + arg + "\"";
    command += " > \"" + logFile.string() + "\" 2>&1\"";

    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // hides console window

    DWORD creationFlags = CREATE_NO_WINDOW; // <- prevents console popup

    if (!CreateProcessA(
            nullptr,
            command.data(),
            nullptr,
            nullptr,
            FALSE,
            creationFlags,
            nullptr,
            workingDir.c_str(),
            &si,
            &pi)) {
        std::cerr << "Failed to launch Python script: " << GetLastError() << "\n";
        running_ = false;
        return;
    }


    processHandle_ = pi.hProcess;
    CloseHandle(pi.hThread);

    fmt::print("Python launched asynchronously. Check log: {}\n", logFile.string());
}

void PythonLauncher::stopWindows() {
    if (processHandle_) {
        TerminateProcess(static_cast<HANDLE>(processHandle_), 0);
        CloseHandle(static_cast<HANDLE>(processHandle_));
        processHandle_ = nullptr;
    }
}
#endif

// ------------------- Unix Implementation -------------------
#ifndef _WIN32
void PythonLauncher::startUnix() {
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Fork failed!\n";
        running_ = false;
        return;
    }

    if (pid == 0) { // Child
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(pythonPath_.c_str()));
        argv.push_back(const_cast<char*>(scriptPath_.c_str()));
        for (auto& arg : args_) argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);

        execvp(pythonPath_.c_str(), argv.data());
        std::cerr << "Failed to exec Python script\n";
        std::exit(1);
    } else { // Parent
        // Parent thread continues without blocking
        childPid_ = pid;
    }
}

void PythonLauncher::stopUnix() {
    if (childPid_ > 0) {
        kill(childPid_, SIGTERM);
        childPid_ = -1;
    }
}
#endif

} // namespace NikTrade
