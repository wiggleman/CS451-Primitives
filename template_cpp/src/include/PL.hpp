#pragma once
#include <mutex>
#include "utility.hpp"
#include <atomic>
#include "FLL.hpp"
//#include <string>

class PL{
  using CallbackType = std::function<void(std::string, Parser::Host)>;
  private:
    
    FLL fll;
    std::vector<Pair> sending; // here host is receiveer
    std::vector<Pair> delivered; //here host is sender
    std::mutex sending_mutex;
    std::mutex delivered_mutex;
    std::atomic<size_t> counter{0};
    CallbackType pl_deliver_nack;
    CallbackType pl_deliver_ack;
    CallbackType pl_deliver_prop;
    std::vector<Parser::Host> hosts;
  public:
    
    PL(Parser::Host host, std::vector<Parser::Host> hosts): fll(host, hosts), hosts(hosts){
      std::thread thread2(&PL::timer_handler, this);
      thread2.detach();
    }

    void send(std::string msg, Parser::Host host){
      
      
      size_t seqNum = counter.fetch_add(1, std::memory_order_relaxed);
      Pair pair = {host, seqNum, msg}; // here host is receiveer
      fll.send(pair.toString(), host);
      //std::cout<< "PL sent " << msg.c_str()<< " to host" << host.id << "\n";
      //std::cout.flush();
      sending_mutex.lock();
      sending.push_back(pair);
      sending_mutex.unlock();
    }

    void subscribe(CallbackType nack, CallbackType ack, CallbackType prop){
      pl_deliver_nack = nack;
      pl_deliver_ack = ack;
      pl_deliver_prop = prop;
      // upon event fll deliver 
      fll.subscribe([this](std::string msg, Parser::Host host) {
          this->fll_deliver(msg, host);
      });
    }

  private:
    void timer_handler() {
      for(;;){
        sending_mutex.lock();
        for (auto &pair : sending){
          fll.send(pair.toString(), pair.host);
        }
        sending_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
    }

    void demultiplexing(std::string msg, Parser::Host host) {
      //std::cout<< "PL recieved "<< msg.c_str()<< " from host" << host.id << "\n";

      if (msg.find(NACK) == 0){
        msg.erase(0, NACK.length());
        pl_deliver_nack(msg,host);
      } else if (msg.find(ACK) == 0){
        msg.erase(0, ACK.length());
        pl_deliver_ack(msg, host);
      } else if(msg.find(PROPOSAL) == 0){
        msg.erase(0, PROPOSAL.length());
        pl_deliver_prop(msg, host);
      }
    }
    
    void fll_deliver(std::string msg, Parser::Host host){
      //std::cout << "fll delivered: "<<msg<<"\n";

      if (msg.find("ACK") == 0){
        
        // this is an acknowledgement, remove the acknowledged message from sending
        msg.erase(0,3);
        Pair tar_pair = Pair::strToPair(msg, hosts);
        sending_mutex.lock();
        for (auto it = sending.begin(); it != sending.end(); ) {
          if (*it == tar_pair) {
            sending.erase(it);  // Remove the element if it matches the target
            break;
          } else {
            ++it;  // Move to the next element in the vector
          }
        }
        sending_mutex.unlock();
      }else{
        // this is a normal message
        Pair tar_pair = Pair::strToPair(msg, hosts);
        tar_pair.host = host; //here host is sender
        delivered_mutex.lock();
        auto result = std::find(delivered.begin(), delivered.end(), tar_pair);
        bool found = (result != delivered.end());
        if (!found){
          delivered.push_back(tar_pair);
        }
        delivered_mutex.unlock();
        if (!found){
          demultiplexing(tar_pair.msg, host);
        }
        std::string prefix = "ACK";
        prefix += msg;
        fll.send(prefix, host);
      }
    }
    
};
