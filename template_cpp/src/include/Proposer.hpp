#pragma once
#include <mutex>
#include "utility.hpp"
#include "PL.hpp"
#include <unordered_map>

class Proposer{

    private:
    struct Snapshot {
        size_t ackCount;
        std::set<unsigned int> proposedValue;
    };

    
    bool active = false;
    size_t ackCount = 0;
    size_t activeProposalNum = 1;
    std::set<unsigned int> proposedValue;
    std::unordered_map<size_t, Snapshot> snapshots;
    std::vector<Parser::Host> hosts;
    size_t fPlusOne;
    PL& pl;

    public:
    Proposer(std::vector<Parser::Host> hosts, PL& pl ): hosts(hosts), pl(pl) {
        fPlusOne = (hosts.size()-1)/2 +1;
    }
    void propose(std::set<unsigned int> proposal){
        proposedValue = proposal;
        active = true; 
        for (auto& host : hosts){
            pl.send( PROPOSAL + encode(activeProposalNum, proposedValue), host);
        }
    }
    void receiveNACK(std::string msg, Parser::Host ){
        if (!active){
            return;
        }
        size_t pNum;
        std::set<unsigned int> values;
        decodeData(msg, pNum, values);
        if (pNum == activeProposalNum){
            snapshots[activeProposalNum] = {ackCount, proposedValue};
            proposedValue.insert(values.begin(), values.end());
            activeProposalNum += 1;
            ackCount = 0;
            for (auto& host : hosts){
                pl.send( PROPOSAL + encode(activeProposalNum, proposedValue), host);
            }
        }
    }
    void receiveACK(std::string msg, Parser::Host ){
        if (!active){
            return;
        }
        std::istringstream iss(msg);
        size_t pNum;
        iss >> pNum;
        if (pNum == activeProposalNum){
            ackCount += 1;
            if (ackCount >= fPlusOne) {
                decide(proposedValue);
                active = false;
            }
        } else{
            snapshots[pNum].ackCount += 1;
            if (snapshots[pNum].ackCount >= fPlusOne) {
                decide(snapshots[pNum].proposedValue);
                active = false;
            }
        }
    }

    private:
    void decide(std::set<unsigned int> values){
        std::cout << "Set elements: ";
        for (const auto& element : values) {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }

};