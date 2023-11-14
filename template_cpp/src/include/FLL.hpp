#pragma once

#define MAXLINE 1024 
#include <functional>
#include <thread>
#include "parser.hpp"


class FLL {
  private:
    int sockfd;
    struct sockaddr_in self_addr;
    std::vector<Parser::Host> hosts;
    bool subscribed = false;

  public:
    using CallbackType = std::function<void(const char *, Parser::Host)>;
    FLL(Parser::Host host, std::vector<Parser::Host> hosts) {
      this->hosts = hosts;

      // Creating socket file descriptor 
      if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
          perror("socket creation failed"); 
          exit(EXIT_FAILURE); 
      } 
      memset(&self_addr, 0, sizeof(self_addr)); 
      // Filling server information 
      self_addr.sin_family    = AF_INET; // IPv4 
      self_addr.sin_addr.s_addr = host.ip;//INADDR_ANY; 
      self_addr.sin_port = host.port; 
      
      // Bind the socket with the server address 
      if ( bind(sockfd, static_cast<const sockaddr*>(static_cast<const void*>(&self_addr)),  
              sizeof(self_addr)) < 0 ) 
      { 
          perror("bind failed"); 
          exit(EXIT_FAILURE); 
      } 


    }
    void send( const char *msg, Parser::Host host) const {
      struct sockaddr_in rcvr_addr;
      //filling rcvr info
      memset(&rcvr_addr, 0, sizeof(rcvr_addr)); 
      rcvr_addr.sin_family    = AF_INET; // IPv4 
      rcvr_addr.sin_addr.s_addr = host.ip;//INADDR_ANY; 
      rcvr_addr.sin_port = host.port; 
      sendto(sockfd, msg, strlen(msg),  0,
      static_cast<const sockaddr*>(static_cast<const void*>(&rcvr_addr)), 
          sizeof(rcvr_addr)); 
      //std::cout << "fl:b " << msg << "\n";

    }
    void subscribe(CallbackType fll_deliver){
      if (!subscribed){
        std::thread thread1(&FLL::recv_msg_handler, this, fll_deliver);
        thread1.detach();
        subscribed = true;
      }
    }

  private:

    void recv_msg_handler(CallbackType callback) const{
      //create buffer/ address structure to use with recvfrom function
      char buffer[MAXLINE]; 
      ssize_t nbytes;
      socklen_t len;
      struct sockaddr_in sndr_addr;
      len = sizeof(sndr_addr); 
      // loop indefinitly, always receving msgs
      for(;;){
        bzero(buffer, sizeof(buffer));
        if ((nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&sndr_addr), &len))==-1){
          perror("recv");
          pthread_exit(NULL);
        }
        Parser::Host host_sndr;
        bool found = false;
        for (auto &host : hosts){
          if (host.port == sndr_addr.sin_port){
            host_sndr = host;
            found = true;
            break;
          }
        }
        if (!found){
          perror("can't identify sender"); 
          pthread_exit(NULL);
        }
        callback(buffer, host_sndr);
      }
    }
};

