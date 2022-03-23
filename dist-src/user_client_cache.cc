#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sys/ioctl.h>
#include <vector>
#include <algorithm>
#include <assert.h>

int on = 1;
int off = 0;

#define LOG_FILE "log/log_file.txt"
#define BUF_SIZE 1024
#define CLI_NUM 1 

#define MAX_HASH_SIZE 16383  // range is 0-16383 

std::vector<int> myList;

int my_hash(std::string str) {

    std::hash<std::string> hash_fn;
    int num = (int) hash_fn(str) % (MAX_HASH_SIZE+1);

#ifdef DEBUG
    std::cout << "hash num" << num << std::endl;
#endif

    return num;
}
// get num-th fd from Mylist
int get_fd_by_num(int user_fd, int num) {
    for(int i = 0; i<num; i++) {
        if(user_fd==myList[i]) {
            return myList[num];
        }
    }
    return myList[num-1];
}

// using hashing and assume the server will not leave the cluster
// we need at least three servers
int pick_client_hash(int user_fd, std::string id) {
//    assert(myList.size()>=2);
    int res = -1;
    int num = my_hash(id);
    if (CLI_NUM == 2) {
        if (num < 8190) {
            // get first fd
            res = get_fd_by_num(user_fd, 1);
        }else {
            res = get_fd_by_num(user_fd, 2);
        }
    }else if (CLI_NUM == 3) {
        if (num < 5500) {
            // get first fd
            res = get_fd_by_num(user_fd, 1);
            //cnt1++;
        }else if (num >=5501 && num <11000) {
            res = get_fd_by_num(user_fd, 2);
            //cnt2++;
        } else {
            res = get_fd_by_num(user_fd, 3);
            //cnt3++;
        }

    } else if(CLI_NUM == 1) {
        res = get_fd_by_num(user_fd, 1);
    } else if(CLI_NUM == 5) {
        if (num < 3300) {
            res = get_fd_by_num(user_fd, 1);
        } else if(num>=3300 && num < 6600) {
            res = get_fd_by_num(user_fd, 2);
        } else if(num>=6600 && num < 9900) {
            res = get_fd_by_num(user_fd, 3);
        } else if(num>=9900 && num < 13200) {
            res = get_fd_by_num(user_fd, 4);
        } else {
            res = get_fd_by_num(user_fd, 5);
        } 
    
    } else if(CLI_NUM == 7) {
        if (num < 2400) {
            res = get_fd_by_num(user_fd, 1);
        } else if(num>=2400 && num < 4800) {
            res = get_fd_by_num(user_fd, 2);
        } else if(num>=4800 && num < 7200) {
            res = get_fd_by_num(user_fd, 3);
        } else if(num>=7200 && num < 9600) {
            res = get_fd_by_num(user_fd, 4);
        } else if(num>=9600 && num < 12000) {
            res = get_fd_by_num(user_fd, 5);
        } else if(num>=12000 && num < 14400) {
            res = get_fd_by_num(user_fd, 6);
        } else {
            res = get_fd_by_num(user_fd, 7);
        }  
    }
    return res;
}



void error_handling(char *message);

void write_log_file( const std::string &text )
{
    std::ofstream log_file(LOG_FILE, std::ios_base::out | std::ios_base::app );
    log_file << text << std::endl;
}

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    // client part: connect to master with specified ip and port
    // if (argc != 3)
    // {
    //     printf("Usage : %s <IP> <port>\n", argv[0]);
    //     exit(1);
    // }

    // sock = socket(PF_INET, SOCK_STREAM, 0);
    // //Enable non-blocking

    // if (sock == -1)
    //     error_handling("socket() error");

    // memset(&serv_adr, 0, sizeof(serv_adr));
    // serv_adr.sin_family = AF_INET;
    // serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    // serv_adr.sin_port = htons(atoi(argv[2]));

    // if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    //     error_handling("connect() error!");
    // else
    //     puts("Connected...........");

    // end of acting as a client
    remove(LOG_FILE);
   
    // act as a server so that client in cluster can connect
    int serv_sock, clnt_sock;
    struct sockaddr_in  clnt_adr;
    socklen_t adr_sz;
    int i;
    char buf[BUF_SIZE];
    socklen_t clnt_adr_sz;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    clnt_adr_sz = sizeof(clnt_adr);

    printf("waiting for the client to connect\n");
    for(i = 0; i<CLI_NUM; i++) {
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        if (clnt_sock == -1)
            error_handling("accept() error");
        else
            printf("Connect client %d \n", i + 1);
            myList.push_back(clnt_sock);
    }

    



    // end of acting as a server setup
        
    //ioctl(sock, FIONBIO, &(on));
 
    int count = 0;
    while (1)
    {
        // fputs("Input message(Q to quit): ", stdout);
        //fgets(message, BUF_SIZE, stdin);
        int num_tests = 100;
        double sum = 0;


        // test get operation
        for(int i = 2; i<num_tests; i++) {
            std::string str = "set "+std::to_string(i)+" 10"; 
            strcpy(message, str.c_str());
            int sock_id = pick_client_hash(100, std::to_string(i));
            
            if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
                break;
            std::string user = "[user]:";
            std::string temp = user + std::string(message);
            std::cout << temp << std::endl;

            //std::cout << temp.c_str() <<std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            
            send(sock_id, temp.c_str(), strlen(temp.c_str()), 0);
            str_len = recv(sock_id, message, BUF_SIZE - 1, 0);
            
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            sum += duration.count();
            // std::cout << duration.count()/num_tests << ", ";
            message[str_len] = 0;
            printf("Message from server: %s\n", message);
        }
        std::cout << "averge set time" << sum/num_tests << std::endl;

        write_log_file("averge set time");
        write_log_file(std::to_string(sum/num_tests));
 
	    sleep(5);
        // get
        sum = 0;
        // test delete operation
        for(int i = 2; i<num_tests; i++) {
            strcpy(message, ("get "+std::to_string(i)).c_str());
            
            if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
                break;
            std::string user = "[user]";
            std::string temp = user + std::string(message);
            std::cout << temp << std::endl;

            int sock_id = pick_client_hash(100, std::to_string(i));

            //std::cout << temp.c_str() <<std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            
            send(sock_id, temp.c_str(), strlen(temp.c_str()), 0);
            str_len = recv(sock_id, message, BUF_SIZE - 1, 0);
            
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            sum += duration.count();
            // std::cout << duration.count()/num_tests << ", ";
            message[str_len] = 0;
            printf("Message from server: %s\n", message);
        }
        std::cout << "averge get time" << sum/num_tests << std::endl;
        write_log_file("averge get time");
        write_log_file(std::to_string(sum/num_tests));
        sleep(5);


        sum = 0;
        // test delete operation
        for(int i = 2; i<num_tests; i++) {
            strcpy(message, ("delete "+std::to_string(i)).c_str());
            int sock_id = pick_client_hash(100, std::to_string(i));
            if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
                break;
            std::string user = "[user]";
            std::string temp = user + std::string(message);
            std::cout << temp << std::endl;

            //std::cout << temp.c_str() <<std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            
            send(sock_id, temp.c_str(), strlen(temp.c_str()), 0);
            str_len = recv(sock_id, message, BUF_SIZE - 1, 0);
            
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            sum += duration.count();
            // std::cout << duration.count()/num_tests << ", ";
            message[str_len] = 0;
            printf("Message from server: %s\n", message);
        }
        std::cout << "averge delete time" << sum/num_tests << std::endl;
        write_log_file("averge delete time");
        write_log_file(std::to_string(sum/num_tests));
        sleep(5);

        count ++;
        if(count==10) break;
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
