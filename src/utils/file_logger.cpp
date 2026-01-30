#include "file_logger.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <unistd.h>
    #include <errno.h>
#endif

// --- Single Instance Logic ---
bool IsAlreadyRunning() {
#ifdef _WIN32
    // Windows Implementation: Named Mutex
    const char* NIKTRADE_MUTEX_NAME = "Global\\NikTrade_SingleInstance_Guard_99";
    HANDLE hMutex = CreateMutexA(NULL, FALSE, NIKTRADE_MUTEX_NAME);
    
    if (hMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND hwnd = FindWindowA(NULL, "NikTrade"); 
        if (hwnd) {
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
        }
        CloseHandle(hMutex);
        return true; 
    }
    return false;

#else
    // Linux/macOS Implementation: Abstract Unix Socket
    // We try to bind to a unique path. If it's taken, the app is running.
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) return false;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/tmp/niktrade.lock", sizeof(addr.sun_path) - 1);

    // Try to bind. If bind fails, another instance has the socket.
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return true; 
    }
    // We leave the socket open! It will close automatically when the process dies.
    return false;
#endif
}

FileLogger::FileLogger(const std::string& filename) {
    logFile_.open(filename, std::ios::app);
}

FileLogger::~FileLogger() {
    if (logFile_.is_open())
        logFile_.close();
}

void FileLogger::logInfo(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!logFile_.is_open()) return;

    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " " << message << "\n";

    logFile_ << oss.str();
    logFile_.flush(); // ensures real-time write
}
