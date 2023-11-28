#pragma once

#include "PL.hpp"

class PFD{
    using CallbackType = std::function<void(Parser::Host)>;

    private:

    std::vector<CallbackType> callbacks;
    std::vector<Parser::Host> correct;
    std::vector<Parser::Host> alive;
    std::mutex callbackMutex;
    std::mutex aliveMutex;
    PL& pl;


    public:
    PFD(std::vector<Parser::Host> hosts, PL &pl):correct(hosts), alive(hosts), pl(pl){
        std::thread thread(&PFD::timer, this);
        thread.detach();
    }
    void pl_deliver_hb(const std::string& msg, Parser::Host host){
        if (msg == "HEARTBEATrequest"){
            pl.send("HEARTBEATreply", host);
        } 
        else if (msg == "HEARTBEATreply"){
            aliveMutex.lock();
            alive.push_back(host);
            aliveMutex.unlock();
        }
    }

    void subscribe(CallbackType callback){
        callbackMutex.lock();
        callbacks.push_back(callback);
        callbackMutex.unlock();
    }

    private:
    void timer(){
        for(;;){

            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            aliveMutex.lock();
            for (auto it = correct.begin(); it != correct.end();) {
                auto result = std::find(alive.begin(), alive.end(), *it);
                bool found = (result != alive.end());
                if (!found) {
                    // it is not alive
                    crash(*it);
                    it = correct.erase(it);
                } else {
                    // alive, send heartbeat request
                    pl.send("HEARTBEATrequest", *it);
                    ++it;
                }
            }
            alive.clear();
            aliveMutex.unlock();
        }
    }
    void crash(Parser::Host host){
        callbackMutex.lock();
        for (auto &callback : callbacks){
            callback(host);
        }
        callbackMutex.unlock();
    }

};