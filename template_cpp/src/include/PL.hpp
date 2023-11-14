#pragma once
#include <mutex>
#include "FLL.hpp"
//#include <string>

class PL{
  using CallbackType = std::function<void(const char *, Parser::Host)>;
  private:
    struct Pair { // a pair of host and msg
      std::string msg;
      Parser::Host host;

      // Overloaded equality operator for the Data struct
      bool operator==(const Pair& other) const {
          return msg == other.msg && host.id == other.host.id;
      }
    };
    FLL fll;
    std::vector<Pair> sending;
    std::vector<Pair> delivered;
    std::mutex sending_mutex;
    std::mutex delivered_mutex;
  public:
    
    PL(Parser::Host host, std::vector<Parser::Host> hosts): fll(host, hosts){
      std::thread thread2(&PL::timer_handler, this);
      thread2.detach();
    }

    void send(const char* msg, Parser::Host host){
      fll.send(msg, host);
      sending_mutex.lock();
      Pair pair = {msg, host};
      sending.push_back(pair);
      sending_mutex.unlock();
      std::cout << "b " << msg << "\n";
      writeToLogFile( std::string("b ") + msg);
    }

    void subscribe(CallbackType pl_deliver){
      // upon event fll deliver 
      fll.subscribe([this, pl_deliver](const char* msg, Parser::Host host) {
          this->fll_deliver(msg, host, pl_deliver);
      });
    }

  private:
    void timer_handler() {
      for(;;){
        sending_mutex.lock();
        for (auto &pair : sending){
          fll.send(pair.msg.c_str(), pair.host);
        }
        sending_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
      

    }
    
    void fll_deliver(const char* msg, Parser::Host host, CallbackType pl_deliver){
        std::string msg_str = msg;
        if (msg_str.find("ACK") == 0){
          // this is an acknowledgement, remove the acknowledged message from sending
          msg_str.erase(0,3);
          Pair tar_pair{msg_str, host};
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
          Pair tar_pair{msg, host};
          bool found = false;
          delivered_mutex.lock();
          for (auto it = delivered.begin(); it != delivered.end(); ) {
            if (*it == tar_pair) {
              found = true;
              break;
            } else {
              ++it;  // Move to the next element in the vector
            }
          }
          if (!found){
            delivered.push_back(tar_pair);
          }
          delivered_mutex.unlock();
          if (!found){
            pl_deliver(msg, host);
          }
          std::string prefix = "ACK";
          prefix += msg;
          fll.send(prefix.c_str(), host);
        }
    }
    
};
