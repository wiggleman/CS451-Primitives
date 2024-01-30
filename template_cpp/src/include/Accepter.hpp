#pragma once
#include <algorithm>
#include "utility.hpp"
#include "PL.hpp"

class Acceptor{
    private:
    std::vector<std::set<unsigned int>> acceptedValue;
    PL& pl;


    public:
    Acceptor(PL& pl, unsigned int p): acceptedValue(p), pl(pl) {}

    void receivePROP(std::string msg, Parser::Host ppsr){
        size_t s;
        size_t pNum;
        std::set<unsigned int> values;
        decodeData(msg, s, pNum, values);

        bool includes = std::includes(values.begin(), values.end(), acceptedValue[s].begin(), acceptedValue[s].end());
        if (includes){
            acceptedValue[s] = values;
            std::ostringstream oss;
            oss << s << " " << pNum << " ";
            std::string encodedPNum = oss.str();
            pl.send( ACK + encodedPNum, ppsr);
        } else{
            acceptedValue[s].insert(values.begin(), values.end());
            pl.send( NACK + encode(s, pNum, acceptedValue[s]), ppsr);
        }
    }
};