#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <sw/redis++/redis++.h>
#include <bits/stdc++.h>
//#include "client.h"

// #define DEBUG

using namespace sw::redis;
using namespace std; 

Redis client = Redis("tcp://127.0.0.1:6379");    
#define BUF_SIZE 1024
void error_handling(char *message);

std::vector<std::string> split(std::string str, char del) {
  std::vector<std::string> internal; 
  std::stringstream ss(str); // Turn the string into a stream. 
  std::string tok; 
 
  while(getline(ss, tok, del)) { 
    internal.push_back(tok); 
  } 
 
  return internal;
}

int fake_set(int id, int obj_size) {
  // std::cout << "inside fake set function" <<std::endl;
  if(id < 0) return -1;
  if(obj_size <=0) return -1;
  // sleep(1);
  return 1;
}

int redis_set(std::string key, std::string val) {
  int status = client.set(key, val);
  
  return status; 
}

int fake_get(int id) {
  // std::cout << "inside fake get function" <<std::endl;
  if(id < 0) return -1;
  // sleep(1);
  return 1;
}


int redis_get(std::string key) {
#ifdef DEBUG
  std::cout << "inside redis get function" << std::endl;
#endif
  auto val = client.get(key);
  int status = -1;
  if (val) {
    status = 1;
  }
  return status;
}

int fake_delete( int id) {
#ifdef DEBUG
  std::cout << "inside fake delete function" << std::endl;
#endif
  char *out;
  size_t size;
  //int status = client.Delete(id);
  return 1;
}

int redis_delete( std::string key) {
#ifdef DEBUG
  std::cout << "inside redis delete function" << std::endl;
#endif
  int status = client.del(key);
  return status;
}

int fake_thru(int id, int obj_size) {
#ifdef DEBUG
  std::cout << "inside fake throughput function" << std::endl;
#endif
  int status1 = fake_set(id, obj_size);

  int status2 = fake_get(id);

  int status3 = fake_delete(id);

  return status1 && status2 && status3;
}



// return status -1, -2, normal number
/*
  -3: incorrect usage of set get commond
  -2: commonad not container [user]
  -1: wrong status from lightning

*/
int process_redis_msg(char *fd, char *message){
//int process_msg(char *fd, char *message) {
  int status =  -1;
  // check if fd exists
  std::vector<std::string> seps = split(message, ':');
  strncpy(fd, (seps[0]).c_str(), (sizeof(fd))-1);  

  if(std::string(message).find("set") != std::string::npos) {
    std::vector<std::string> sep = split(message, ' ');
    if(sep.size() != 3) {
      return -3;
    }
    // auto start = std::chrono::high_resolution_clock::now(); 
    status = fake_set(std::stoi(sep[1]), std::stoi(sep[2]));
    // auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> duration = end - start;
    // std::cout << "redis set time" << duration.count() << std::endl;

  } else if(std::string(message).find("get") != std::string::npos) {

    std::vector<std::string> sep = split(message, ' ');
    if(sep.size() != 2) {
      return -3;
    }
    // status = redis_get(sep[1]);
    status = fake_get(std::stoi(sep[1]));
  } else if(std::string(message).find("del") != std::string::npos) {

    std::vector<std::string> sep = split(message, ' ');
    if(sep.size() != 2) {
      return -3;
    }
    // status = redis_delete(sep[1]);
    status = fake_delete(std::stoi(sep[1]));
  } else if(std::string(message).find("thru") != std::string::npos) {
    std::vector<std::string> sep = split(message, ' ');
    if(sep.size() != 3) {
      return -3;
    }
    status = fake_thru(std::stoi(sep[1]), std::stoi(sep[2]));
    //status = fake_delete(std::stoi(sep[1]));
  
  } else if(std::string(message).find("mput") != std::string::npos) {

  } else if(std::string(message).find("mget") != std::string::npos) {

  } else if(std::string(message).find("mupdate") != std::string::npos) {

  } else {
    std::cout << "unrecognized commond from master" <<std::endl;
    return -2;
  }

  return status;
}

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("connect() error!");
    else
        puts("Connected...........");

    //LightningClient client("/tmp/lightning", "password");

    while (1)
    {
      // fputs("Input message(Q to quit): ", stdout);
      // fgets(message, BUF_SIZE, stdin);

      if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        break;

      // write(sock, message, strlen(message));
      str_len = recv(sock, message, BUF_SIZE - 1, 0);
      //std::cout << "recving from server " << str_len << std::endl; 
      message[str_len] = 0;
      // printf("Message from server: %s", message);
      // todo check if contains user message
      if ( std::string(message).find("[user]") != std::string::npos)
      {
#ifdef DEBUG
          std::cout << "processing the message and interacting with lightning." << std::endl;
          std::cout << message << std::endl;
#endif           
          // calling  lightning api to process the message
          int status = -1;
          char state[4];
          char fd[] = "placeholder"; 
          status = process_redis_msg(fd, message);
#ifdef DEBUG
          std::cout << "user fd: " << fd << " fd len: " << strlen(fd) << std::endl;
#endif
          if (status < 0) {
          //return error status
            sprintf(state, "%d", status);
            // send state and fd and it will be fd:state
            std::string new_state = std::string(fd) + ":"+ std::string(state);
            send(sock, new_state.c_str(), std::strlen(new_state.c_str()), 0);
          } else {
            sprintf(state, "%d", status);
            std::string new_state = std::string(fd) + ":"+ std::string(state);
            send(sock, new_state.c_str(), std::strlen(new_state.c_str()), 0);

          }
// #ifdef DEBUG
            std::cout << "send is finished" << std::endl;
// #endif
      } else {
          std::cout << "incorrect message from server" << std::endl; 
      }
    }
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
