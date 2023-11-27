#include "parser.hpp"
#include "run.hpp"
#include "PL.hpp"
#include "BEB.hpp"
#include "PFD.hpp"
#include "utility.hpp"
#include "URB.hpp"


void callback(const std::string& msg, Parser::Host host_sndr){
    std::cout << "d " << host_sndr.id << " " << msg << "\n";
    writeToLogFile(std::string("d ") + std::to_string(host_sndr.id) + " " + msg);
    std::cout.flush();
} 

void PFDCallback(Parser::Host host_crashed){
    std::cout << "host " << host_crashed.id << " crashed!" << "\n";
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
    unsigned long M;
    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    if (!(iss >> M)) {
        std::cerr << "Error parsing line: " << line << std::endl;
        return;
    }
    
    file.close();
    //std::cout << n << " " << M << " " << i;

    Parser::Host host_me = hosts[parser.id() - 1];
    PL* pl = new PL(host_me, hosts);
    BEB* beb = new BEB(hosts, *pl);
    PFD* pfd = new PFD(hosts, *pl);
    URB* urb = new URB(host_me, hosts, *beb);

    urb -> subscribe(callback);
    pfd -> subscribe([urb](Parser::Host host) {
            urb -> pfd_notify(host);
        });
    pl -> subscribe( [beb](std::string msg, Parser::Host host) {
            beb -> pl_deliver(msg, host);
        },
        [pfd](std::string msg, Parser::Host host) {
            pfd -> pl_deliver_hb(msg, host);
        }
    );
    
    for (unsigned long m = 0; m < M; m++){
        urb->broadcast(std::to_string(m+1).c_str() );
    }
    
        
    std::cout.flush();

    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
    delete beb;
    delete urb;
    delete pl;
    delete pfd;
}

// void receive(std::vector<Parser::Host> hosts){

// }
