#pragma once
#include "parser.hpp"

extern std::ofstream logFile;
void writeToLogFile(const std::string& message);

struct Pair { // a pair of host and msg
    Parser::Host host; //sometimes it signifies the sender sometimes the receiver
    size_t seqNum;
    std::string msg;

    // Overloaded equality operator for the Data struct
    bool operator==(const Pair& other) const {
        return seqNum == other.seqNum && host.id == other.host.id;
    }
        // Function to encode Pair to a string
    std::string toString() const {
        std::ostringstream oss;
        oss << host.id  << "," << seqNum << "," << msg;
        return oss.str();
    }

    // Function to decode a string to Pair
    static Pair strToPair(const std::string& encodedStr, const std::vector<Parser::Host>& hosts) {
        Pair result;

        // Split the encoded string into msg and id
        std::istringstream iss(encodedStr);
        std::string idStr;
        std::string seqStr;
        std::getline(iss, idStr, ',');
        std::getline(iss, seqStr, ',');
        std::getline(iss, result.msg );

        // Convert id string to size_t
        size_t id = std::stoi(idStr);
        result.seqNum = std::stoi(seqStr);

        // Check if there is a matching host in the vector
        bool found = false;
        for (auto &host : hosts){
            if (host.id == id){
                result.host = host;
                found = true;
                break;
            }
        }

        // If a matching host is found, return the Pair; otherwise, handle the error as needed
        if (found) {
            return result;
        } else {
            perror("error when decode a string to Pair"); 
            exit(EXIT_FAILURE); 
        }
    }
};

namespace std {
    template <>
    struct hash<Pair> {
        size_t operator()(const Pair& p) const {
            return hash<size_t>()(p.seqNum) ^ hash<unsigned long>()(p.host.id);
        }
    };
    template <>
    struct hash<Parser::Host> {
        size_t operator()(const Parser::Host& host) const {
            return hash<unsigned long>()(host.id);
        }
    };
}

