#pragma once
#include <string>
#include <mutex>
#include <fstream>

class FileLogger {
public:
    explicit FileLogger(const std::string& filename);
    ~FileLogger();

    void logInfo(const std::string& message);

private:
    std::ofstream logFile_;
    std::mutex mutex_;
};
