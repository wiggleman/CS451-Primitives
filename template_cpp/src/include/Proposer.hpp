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

    
    std::vector<bool> active;
    std::vector<bool> decided;
    std::vector<size_t> ackCount;
    std::vector<size_t> activeProposalNum;
    std::vector<std::set<unsigned int>> proposedValue;
    std::vector<std::set<unsigned int>> decidedValue;
    std::vector<std::unordered_map<size_t, Snapshot>> snapshots;
    std::vector<Parser::Host> hosts;
    size_t fPlusOne;
    PL& pl;

    public:
    Proposer(std::vector<Parser::Host> hosts, PL& pl, unsigned int p): 
        active(p, false), decided(p,false), ackCount(p, 0), activeProposalNum(p, 1), decidedValue(p), snapshots(p),
        hosts(hosts), pl(pl) 
    {
        fPlusOne = (hosts.size()-1)/2 +1;
    }
    void propose(std::set<unsigned int> proposal){
        proposedValue.push_back(proposal);
        size_t s = proposedValue.size() - 1;
        active[s] = true; 
        for (auto& host : hosts){
            pl.send( PROPOSAL + encode(s, activeProposalNum[s], proposedValue[s]), host);
        }
    }
    void receiveNACK(std::string msg, Parser::Host ){

        size_t s;
        size_t pNum;
        std::set<unsigned int> values;
        decodeData(msg, s, pNum, values);
        if (!active[s]){
            return;
        }
        if (pNum == activeProposalNum[s]){
            snapshots[s][activeProposalNum[s]] = {ackCount[s], proposedValue[s]};
            proposedValue[s].insert(values.begin(), values.end());
            activeProposalNum[s] += 1;
            ackCount[s] = 0;
            for (auto& host : hosts){
                pl.send( PROPOSAL + encode(s, activeProposalNum[s], proposedValue[s]), host);
            }
        }
    }
    void receiveACK(std::string msg, Parser::Host ){

        std::istringstream iss(msg);
        size_t s;
        size_t pNum;
        iss >> s >> pNum;
        if (!active[s]){
            return;
        }
        if (pNum == activeProposalNum[s]){
            ackCount[s] += 1;
            if (ackCount[s] >= fPlusOne) {
                decide(s, proposedValue[s]);
                active[s] = false;
            }
        } else{
            snapshots[s][pNum].ackCount += 1;
            if (snapshots[s][pNum].ackCount >= fPlusOne) {
                decide(s, snapshots[s][pNum].proposedValue);
                active[s] = false;
            }
        }
    }

    private:
    void decide(size_t s, std::set<unsigned int> values){
        decidedValue[s] = values;
        decided[s] = true;
        bool allTrue = std::all_of(decided.begin(), decided.end(), [](bool value) { return value; });
        if (allTrue){
            for (const auto& mySet : decidedValue) {
                std::ostringstream oss;
                for (const auto& element : mySet) {
                    oss << element << " ";
                }
                writeToLogFile(oss.str());
            }
        }

    }

};