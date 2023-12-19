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

std::string encode(const size_t& pNum, const std::set<unsigned int>& values) {
    // Encode size_t to string
    std::ostringstream oss;
    oss << pNum;
    std::string encodedPNum = oss.str();

    // Encode set to string
    std::ostringstream setOss;
    for (const auto& element : values) {
        setOss << element << " ";
    }
    std::string encodedValues = setOss.str();

    return encodedPNum + "|" + encodedValues;
}

// Decoder function
void decodeData(const std::string& encodedString, size_t& pNum, std::set<unsigned int>& values) {
    // Find the position of the separator
    size_t separatorPos = encodedString.find('|');

    std::string encodedPNum = encodedString.substr(0, separatorPos);
    std::string encodedValues = encodedString.substr(separatorPos + 1);

    std::istringstream iss(encodedPNum);
    iss >> pNum;

    std::istringstream setIss(encodedValues);
    unsigned int element;
    while (setIss >> element) {
        values.insert(element);
    }
}