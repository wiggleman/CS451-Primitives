#include "parser.hpp"
#include "run.hpp"
#include "PL.hpp"
#include "Proposer.hpp"
#include "Accepter.hpp"
#include "utility.hpp"



void callback(const std::string& msg, Parser::Host host_sndr){
    std::cout << "d " << host_sndr.id << " " << msg << "\n";
    writeToLogFile(std::string("d ") + std::to_string(host_sndr.id) + " " + msg);
    std::cout.flush();
} 


void run(Parser parser, std::vector<Parser::Host> hosts){
    logFile.open(parser.outputPath());

    const char *config_path = parser.configPath();
    std::ifstream file(config_path);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }
    size_t n = hosts.size();
    unsigned int p;
    unsigned int vs;
    unsigned int ds;
    std::vector<std::set<unsigned int>> proposals;
    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    iss >> p >> vs >> ds;
    std::cout << "p: " << p << "vs: " << vs << "ds: "<< ds;
    for (unsigned int i = 0; i < p; ++i) {
        std::getline(file, line);
        std::set<unsigned int> proposal;
        std::istringstream iss(line);
        unsigned int element;
        while (iss >> element) {
            proposal.insert(element);
        }
        proposals.push_back(proposal);
    }
    
    file.close();
    //std::cout << n << " " << M << " " << i;

    Parser::Host host_me = hosts[parser.id() - 1];
    PL* pl = new PL(host_me, hosts);
    Proposer* ppsr = new Proposer(hosts, *pl);
    Acceptor* acptr = new Acceptor(*pl);
    pl -> subscribe([ppsr](std::string msg, Parser::Host host) {
            ppsr -> receiveNACK(msg, host);
        },[ppsr](std::string msg, Parser::Host host) {
            ppsr -> receiveACK(msg, host);
        },[acptr](std::string msg, Parser::Host host) {
            acptr -> receivePROP(msg, host);
        }
    );

 
    
    for (auto& proposal: proposals){
        ppsr -> propose(proposal);
        break;
    }
    
        
    std::cout.flush();

    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
    delete pl;
    delete ppsr;
    delete acptr;
}

// void receive(std::vector<Parser::Host> hosts){

// }
