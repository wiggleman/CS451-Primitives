#pragma once
#include "utility.hpp"


class FIFO {
    using CallbackType = std::function<void(std::string, Parser::Host)>;

    private:
    URB &urb;
    size_t lsn = 0;
    Parser::Host self;
    std::vector<Parser::Host> hosts;
    std::unordered_map<Parser::Host, size_t> next;
    std::unordered_map<Parser::Host, std::vector<Pair>> pending;
    CallbackType fifo_deliver;
    
    public:
    FIFO(Parser::Host host, std::vector<Parser::Host> hosts, URB &urb): urb(urb), self(host), hosts(hosts) {
        for (auto& h:hosts){
            next[h] = 1;
        }
    }

    void broadcast(std::string msg){
        lsn += 1;
        Pair pair = {self, lsn, msg};
        urb.broadcast(pair.toString());
        std::cout << "b " << msg << "\n";
        writeToLogFile( std::string("b ") + msg);
    }

    void subscribe(CallbackType fifo_deliver){
        this->fifo_deliver = fifo_deliver;
        urb.subscribe([this](std::string msg, Parser::Host host) {
          this->urb_deliver(msg, host);
      });
    }

    private:
    void urb_deliver(std::string msg, Parser::Host p){
        Pair pair = Pair::strToPair(msg, hosts);
        Parser::Host sndr = pair.host;
        pending[sndr].push_back(pair);
        auto pairComparator = [](const Pair& a, const Pair& b) {
            return a.seqNum < b.seqNum;
        };
        std::sort(pending[sndr].begin(), pending[sndr].end(), pairComparator);
        for (auto it = pending[sndr].begin(); it != pending[sndr].end();) {
            if (it->seqNum == next[sndr]) {
                next[sndr] += 1;
                fifo_deliver(it->msg, it->host);
                it = pending[sndr].erase(it);
            } else {
                break;
            }
        }
    }
};