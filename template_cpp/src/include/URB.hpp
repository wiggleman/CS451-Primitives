#pragma once
#include "BEB.hpp"
#include "utility.hpp"
#include <unordered_map>
#include <algorithm>

class URB {

    using CallbackType = std::function<void(std::string, Parser::Host)>;

    private:
    BEB &beb;
    std::vector<Parser::Host> correct;
    std::vector<Parser::Host> hosts;
    std::mutex correctMutex;
    std::mutex pendingMutex;
    std::mutex deliveredMutex;
    std::mutex ackMutex;
    std::vector<Pair> delivered; // here host is original sender
    std::vector<Pair> pending; // here host is original sender
    std::unordered_map<Pair, std::vector<Parser::Host>> ack;
    CallbackType urb_deliver;
    std::atomic<size_t> counter{0};

    Parser::Host self;
    
    public:
    URB(Parser::Host host, std::vector<Parser::Host> hosts, BEB &beb): beb(beb), correct(hosts),hosts(hosts), self(host){
    }

    void broadcast(std::string msg){
        size_t seqNum = counter.fetch_add(1, std::memory_order_relaxed);
        Pair pair = {self, seqNum, msg};
        pendingMutex.lock();
        pending.push_back(pair);
        pendingMutex.unlock();
        beb.broadcast(pair.toString());

    }

    void subscribe(CallbackType urb_deliver){
        this->urb_deliver = urb_deliver;
        beb.subscribe([this](std::string msg, Parser::Host host) {
          this->beb_deliver(msg, host);
      });
    }

    void pfd_notify(Parser::Host crashed){
        std::cout << "host " << crashed.id << " crashed!" << "\n";
        correctMutex.lock();
        for (auto it = correct.begin(); it != correct.end();) {
            if (*it == crashed) {
                it = correct.erase(it);
                break;
            } else {
                ++it;
            }
        }
        correctMutex.unlock();
        pendingMutex.lock();
        for (auto it = pending.begin(); it != pending.end();) {
            if (can_deliver(*it)) {
                deliver(*it);
                it = pending.erase(it);
            } else {
                ++it;
            }
        }
        pendingMutex.unlock();
    }

    private:
    void beb_deliver(std::string msg, Parser::Host sender){
        Pair pair = Pair::strToPair(msg,hosts);
        ack[pair].push_back(sender);
        pendingMutex.lock();
        auto result = std::find(pending.begin(), pending.end(), pair);
        bool found= (result != pending.end());
        if (!found){
            pending.push_back(pair);
            beb.broadcast(pair.toString());
            result = pending.end() - 1;
        }
        
        if (can_deliver(pair)){
            deliver(pair);
            pending.erase(result);  
        }
        pendingMutex.unlock();
        
    }
    bool can_deliver(const Pair& pair){ // canDeliver can only be called by beb_deliver and pfd_notify, i.e. from different threads
        auto& ackM = ack[pair];
        auto hostComparator = [](const Parser::Host& host1, const Parser::Host& host2) {
            return host1.id < host2.id;  
        };
        correctMutex.lock();
        ackMutex.lock();
        std::sort(ackM.begin(), ackM.end(), hostComparator);
        std::sort(correct.begin(), correct.end(), hostComparator);
        bool include =  std::includes(ackM.begin(), ackM.end(),
                         correct.begin(), correct.end(), hostComparator);
        correctMutex.unlock();
        ackMutex.unlock();
        return include;
    }
    void deliver(const Pair& pair){
        deliveredMutex.lock();
        auto result = std::find(delivered.begin(), delivered.end(), pair);
        bool found= (result != delivered.end());
        if (!found){
            delivered.push_back(pair);
        }
        deliveredMutex.unlock();
        if (!found){
            urb_deliver(pair.msg, pair.host);
        }
    }


};