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
#include <thread>

int on = 1;
int off = 0;

#define LOG_FILE "log-thru/log_file.txt"
#define BUF_SIZE 1024
void error_handling(char *message);

void write_log_file( const std::string &text )
{
    std::ofstream log_file(LOG_FILE, std::ios_base::out | std::ios_base::app );
    log_file << text << std::endl;
}

int num_tests = 40;

void sendMsg(int sock) {
    char message[BUF_SIZE];
    for(int i = 2; i<num_tests; i++) {
        std::string str = "thru "+std::to_string(i)+" 1024"; 
        strcpy(message, str.c_str());
        
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;
        std::string user = "[user]:";
        std::string temp = user + std::string(message);
        std::cout << temp << std::endl;

        send(sock, temp.c_str(), strlen(temp.c_str()), 0);
        
    }
}

void recvMsg(int sock) {
    char message[BUF_SIZE];
    int str_len;
    int counter = 0;
    for(;;) {

        str_len = recv(sock, message, BUF_SIZE - 1, 0);
        if(str_len>0) counter++;
        message[str_len] = 0;
        printf("Message from server: %s\n", message);
        std::cout << counter << std::endl;
        if(counter==num_tests-2) break;
    }
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
    //Enable non-blocking

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

    remove(LOG_FILE);
   
        
    ioctl(sock, FIONBIO, &(on));
 
    int count = 0;
    while (1)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::thread send_thr (sendMsg, sock);
        std::thread recv_thr (recvMsg, sock);
        
        send_thr.join();
        recv_thr.join();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "throughput lightning" << num_tests/duration.count() << std::endl;


        //fgets(message, BUF_SIZE, stdin);
        
        // for(int i = 2; i<num_tests; i++) {
        //     std::string str = "thru "+std::to_string(i)+" 1024"; 
        //     strcpy(message, str.c_str());
            
        //     if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        //         break;
        //     std::string user = "[user]:";
        //     std::string temp = user + std::string(message);
        //     std::cout << temp << std::endl;

        //     //std::cout << temp.c_str() <<std::endl;
    
        //     send(sock, temp.c_str(), strlen(temp.c_str()), 0);
        //     str_len = recv(sock, message, BUF_SIZE - 1, 0);
             
        //     // std::cout << duration.count()/num_tests << ", ";
        //     message[str_len] = 0;
        //     printf("Message from server: %s\n", message);
        // }
        // auto end = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> duration = end - start;
        // sum += duration.count();
        // std::cout << "throughput lightning" << num_tests/sum << std::endl;

        // write_log_file("Lightning throughput");
        // write_log_file(std::to_string(num_tests/sum));
 
	    sleep(5);

        // sum = 0;
        // // test delete operation
        // for(int i = 2; i<num_tests; i++) {
        //     strcpy(message, ("delete "+std::to_string(i)).c_str());
            
        //     if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        //         break;
        //     std::string user = "[user]";
        //     std::string temp = user + std::string(message);
        //     std::cout << temp << std::endl;

        //     //std::cout << temp.c_str() <<std::endl;
        //     auto start = std::chrono::high_resolution_clock::now();
            
        //     send(sock, temp.c_str(), strlen(temp.c_str()), 0);
        //     str_len = recv(sock, message, BUF_SIZE - 1, 0);
            
        //     auto end = std::chrono::high_resolution_clock::now();
        //     std::chrono::duration<double> duration = end - start;
        //     sum += duration.count();
        //     // std::cout << duration.count()/num_tests << ", ";
        //     message[str_len] = 0;
        //     printf("Message from server: %s\n", message);
        // }
        // std::cout << "averge delete time" << sum/num_tests << std::endl;
        // write_log_file("averge delete time");
        // write_log_file(std::to_string(sum/num_tests));
        // sleep(5);

        count ++;
        if(count==1) break;
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
