#include "file_logger.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

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
