#include "utility.hpp"
#include <mutex>

std::mutex fileMutex;
std::ofstream logFile; // Open the file in append mode
const std::string PROPOSAL = "PROPOSAL";
const std::string ACK = "ACK";
const std::string NACK = "NACK";

void writeToLogFile(const std::string& message) {
    // Lock the mutex before writing to the file
    std::lock_guard<std::mutex> lock(fileMutex);

    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
}

std::string encode(size_t v1, size_t v2, const std::set<unsigned int>& v3) {
    std::ostringstream oss;
    oss << v1 << " " << v2 << " ";

    // Serialize the set elements
    for (const auto& element : v3) {
        oss << element << " ";
    }

    return oss.str();
}

// Decoder function
void decodeData(const std::string& encoded, size_t& v1, size_t& v2, std::set<unsigned int>& v3) {
    std::istringstream iss(encoded);
    iss >> v1 >> v2;

    // Deserialize the set elements
    unsigned int element;
    while (iss >> element) {
        v3.insert(element);
    }
    
}