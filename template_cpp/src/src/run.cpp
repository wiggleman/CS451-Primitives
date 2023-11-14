#include "parser.hpp"
#include "run.hpp"
#include "PL.hpp"

std::mutex fileMutex;
std::ofstream logFile; // Open the file in append mode

void writeToLogFile(const std::string& message) {
    // Lock the mutex before writing to the file
    std::lock_guard<std::mutex> lock(fileMutex);

    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
}

void callback(const char *msg, Parser::Host host_sndr){
    std::cout << "d " << host_sndr.id << " " << msg << "\n";
    writeToLogFile(std::string("d ") + std::to_string(host_sndr.id) + " " + msg);
    std::cout.flush();
} 

void run(Parser parser, std::vector<Parser::Host> hosts){
    logFile.open(parser.outputPath(), std::ios::app);

    const char *config_path = parser.configPath();
    std::ifstream file(config_path);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }
    size_t n = hosts.size();
    unsigned long M, i;
    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    if (!(iss >> M >> i)) {
        std::cerr << "Error parsing line: " << line << std::endl;
        return;
    }
    
    file.close();
    //std::cout << n << " " << M << " " << i;

    Parser::Host host_me = hosts[parser.id() - 1];
    PL* pl = new PL(host_me, hosts);
    pl -> subscribe(callback);

    if (parser.id() != i){
        Parser::Host host_rcvr = hosts[i-1];
        for (unsigned long m = 0; m < M; m++){
            pl->send(std::to_string(m+1).c_str(), host_rcvr);
        }
    }
        
    std::cout.flush();

    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
    delete pl;
}

// void receive(std::vector<Parser::Host> hosts){

// }
