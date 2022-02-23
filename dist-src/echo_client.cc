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

//#include "client.h"
#define DEBUG

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
  std::cout << "inside fake set function" <<std::endl;
  if(id < 0) return -1;
  if(obj_size <=0) return -1;
  // sleep(1);
  return 1;
}

//int light_set(LightningClient &client, int id, int obj_size) {
//  char *a = new char[obj_size];
//  for (int i = 0; i < obj_size; i++) {
//    a[i] = 'a';
//  }
//  uint8_t *ptr;
//  int status = client.Create(id, &ptr, obj_size);
//  memcpy(ptr, a, obj_size);
//  status = client.Seal(id);
//  
//  delete[] a;
//  return status; 
//}

int fake_get(int id) {
  std::cout << "inside fake get function" <<std::endl;
  if(id < 0) return -1;
  // sleep(1);
  return 1;
}


//int light_get(LightningClient &client, int id) {
//#ifndef DEBUG
//  std::cout << "inside lightning get function" << std::endl;
//#endif
//  char *out;
//  size_t size;
//  int status = client.Get(id, (uint8_t **)&out, &size);
//  return status;
//}
//int fake_delete(int id) {
//  std::cout << "inside fake delete function" <<std::endl;
//  if(id < 0) return -1;
//  // sleep(1);
//  return 1;
//}


int fake_delete( int id) {
#ifdef DEBUG
  std::cout << "inside fake delete function" << std::endl;
#endif
  char *out;
  size_t size;
  //int status = client.Delete(id);
  return 1;
}

//int light_delete(LightningClient &client, int id) {
//#ifndef DEBUG
//  std::cout << "inside lightning delete function" << std::endl;
//#endif
//  char *out;
//  size_t size;
//  int status = client.Delete(id);
//  return status;
//}



// return status -1, -2, normal number
/*
  -3: incorrect usage of set get commond
  -2: commonad not container [user]
  -1: wrong status from lightning

*/
//int process_msg(char *fd, LightningClient &client, char *message){
int process_msg(char *fd, char *message) {
  int status =  -1;
  // check if fd exists
  std::vector<std::string> seps = split(message, ':');
  strncpy(fd, (seps[0]).c_str(), (sizeof(fd))-1);  

  if(std::string(message).find("set") != std::string::npos) {
    std::vector<std::string> sep = split(message, ' ');
    if(sep.size() != 3) {
      return -3;
    }
    // for(std::string t : sep) {
    //   std::cout << t << std::endl;
    // }
    // std::cout << std::stoi(sep[2]) << std::endl;
    status = fake_set(std::stoi(sep[1]), std::stoi(sep[2]));
    //status = light_set(client, std::stoi(sep[1]), std::stoi(sep[2]));
  } else if(std::string(message).find("get") != std::string::npos) {

    std::vector<std::string> sep = split(message, ' ');
    if(sep.size() != 2) {
      return -3;
    }
    //status = light_get(client, std::stoi(sep[1]));
    status = fake_get(std::stoi(sep[1]));
  } else if(std::string(message).find("delete") != std::string::npos) {

    std::vector<std::string> sep = split(message, ' ');
    if(sep.size() != 2) {
      return -3;
    }
    //status = light_delete(client, std::stoi(sep[1]));
    status = fake_delete(std::stoi(sep[1]));
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
      printf("Message from server: %s", message);
      // todo check if contains user message
      if ( std::string(message).find("[user]") != std::string::npos)
      {
          std::cout << "processing the message and interacting with lightning." << std::endl;
          std::cout << message << std::endl;
           
          // calling  lightning api to process the message
          int status = -1;
          char state[4];
          char fd[] = "placeholder"; 
          status = process_msg(fd, message);
          std::cout << "user fd: " << fd << " fd len: " << strlen(fd) << std::endl;
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
            std::cout << "send is finished" << std::endl;
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
