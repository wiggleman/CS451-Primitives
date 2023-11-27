#include "utility.hpp"
#include <mutex>

std::mutex fileMutex;
std::ofstream logFile; // Open the file in append mode

void writeToLogFile(const std::string& message) {
    // Lock the mutex before writing to the file
    std::lock_guard<std::mutex> lock(fileMutex);

    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
}

