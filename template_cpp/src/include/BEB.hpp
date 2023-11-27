#pragma once
#include "PL.hpp"


class BEB{
    using CallbackType = std::function<void(std::string, Parser::Host)>;

    private:
    PL &pl;
    std::vector<Parser::Host> hosts;
    CallbackType beb_deliver;

    public:
    BEB(std::vector<Parser::Host> hosts, PL &pl): pl(pl), hosts(hosts){}
    void broadcast(std::string msg){
        for (auto &host : hosts){
            pl.send(msg, host);
        }
        //std::cout << "beb";

    }
    void subscribe(CallbackType beb_deliver){
        this->beb_deliver = beb_deliver;
    }
    void pl_deliver(std::string msg, Parser::Host host){
        beb_deliver(msg, host);
    }

};