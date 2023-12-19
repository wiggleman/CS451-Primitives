#pragma once
#include <algorithm>
#include "utility.hpp"
#include "PL.hpp"

class Acceptor{
    private:
    std::set<unsigned int> acceptedValue;
    PL& pl;


    public:
    Acceptor(PL& pl):pl(pl) {}

    void receivePROP(std::string msg, Parser::Host ppsr){
        size_t pNum;
        std::set<unsigned int> values;
        decodeData(msg, pNum, values);

        bool includes = std::includes(values.begin(), values.end(), acceptedValue.begin(), acceptedValue.end());
        if (includes){
            acceptedValue = values;
            std::ostringstream oss;
            oss << pNum;
            std::string encodedPNum = oss.str();
            pl.send( ACK + encodedPNum, ppsr);
        } else{
            acceptedValue.insert(values.begin(), values.end());
            pl.send( NACK + encode(pNum, acceptedValue), ppsr);
        }
    }
};