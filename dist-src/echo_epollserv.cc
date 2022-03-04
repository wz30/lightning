#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <iostream>
#include <thread>
#include <list>
#include <vector>
#include <algorithm>
#include <mutex>
#include <unistd.h>
#include <assert.h>
#include <sstream>
#include <sys/ioctl.h>
int on = 1;
int off = 1;

#define MAX_HASH_SIZE 16383  // range is 0-16383 
#define BUF_SIZE 100
#define EPOLL_SIZE 50

#define DEBUG
#define CLI_NUM 2
// assume two lightning clients: 0-8190 8191-16383
// assume threee lightning clients: 0-5500, 5501-11000, 11001-16383

using namespace std;
// a global variable tn hold fd
std::vector<int> myList;

void error_handling(char *message);

// get hash number between 0-MAX_HASH_SIZE  
int my_hash(std::string str) {

    std::hash<std::string> hash_fn;
    int num = (int) hash_fn(str) % (MAX_HASH_SIZE+1);

    #ifndef DEBUG
    std::cout << "hash num" << num << std::endl;
    #endif

    return num;
}


void printList()
{
//    std::lock_guard<std::mutex> guard(myMutex);
    std::cout << "print a vector of fds"<< std::endl;
    while(1) {
        for(auto itr = myList.begin(), end_itr = myList.end(); itr != end_itr; ++itr) {
            cout << *itr << ",";
        }
        cout << endl;
        sleep(5);
    }
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
    assert(myList.size()>=2);
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
        }else if (num >=5501 && num <11000) {
            res = get_fd_by_num(user_fd, 2);
        } else {
            res = get_fd_by_num(user_fd, 3);
        }

    } else if(CLI_NUM == 1) {
        res = get_fd_by_num(user_fd, 1);
    }
    return res;
}

// todo with consistent hashing
int pick_client(int user_fd)
{
    int res = 0;
    int ind = rand() % (myList.size());
    res = myList[ind];
    do{
        ind = rand() % (myList.size());
        res = myList[ind];
    }while(user_fd==res);
#ifdef DEBUG
    std::cout << "picked fd: " << res << std::endl;
#endif
    return res;  
}
std::vector<std::string> split(std::string str, char del) {
    std::vector<std::string> internal; 
    std::stringstream ss(str); // Turn the string into a stream. 
    std::string tok; 
 
    while(getline(ss, tok, del)) { 
        internal.push_back(tok); 
    } 
 
    return internal;
}

int get_id(std::string buf) {
    // [user]:set 1 100
    std::vector<std::string> sep = split(buf, ' ');
    // todo check status
    return std::stoi(sep[1]); 
}

//std::vector<std::string> split(std::string str, char del) {
//  std::vector<std::string> internal; 
//  std::stringstream ss(str); 
//  std::string tok; 
// 
//  while(getline(ss, tok, del)) { 
//    internal.push_back(tok); 
//  } 
// 
//  return internal;
//}


int main(int argc, char *argv[])
{
    srand(getpid());
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    int str_len, i;
    char buf[BUF_SIZE];

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    if (argc != 2)
    {
        printf("Usage : %s <port> \n", argv[0]);
        exit(1);
    }
    
    // start thread to print list
    std::thread t1(printList);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    //Enable non blocking
    ioctl(serv_sock, FIONBIO, &(on));

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    epfd = epoll_create(EPOLL_SIZE); //可以忽略这个参数，填入的参数为操作系统参考
    ep_events = (epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.events = EPOLLIN; //需要读取数据的情况
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event); //例程epfd 中添加文件描述符 serv_sock，目的是监听 enevt 中的事件

    while (1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1); //获取改变了的文件描述符，返回数量
        if (event_cnt == -1)
        {
            puts("epoll_wait() error");
            break;
        }
	
	    std::cout << event_cnt << std::endl;
        for (i = 0; i < event_cnt; i++)
        {
            if (ep_events[i].data.fd == serv_sock) //客户端请求连接时
            {
                adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
                myList.push_back(clnt_sock);
                sort( myList.begin(), myList.end() );
                myList.erase( unique( myList.begin(), myList.end() ), myList.end() );

                event.events = EPOLLIN;
                event.data.fd = clnt_sock; //把客户端套接字添加进去
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("connected client : %d \n", clnt_sock);
            }
            else //是客户端套接字时
            {
		        memset(buf, 0, BUF_SIZE);

                auto start = std::chrono::high_resolution_clock::now();
                str_len = recv(ep_events[i].data.fd, buf, BUF_SIZE, 0);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                std::cout <<"receive message from user/client time: " << duration.count() << std::endl;
                
#ifdef DEBUG
		        std::cout << "message from client: " << buf <<" len: " <<str_len << std::endl;
#endif
                if (str_len == 0)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL); //从epoll中删除套接字
                    myList.erase(std::remove(myList.begin(), myList.end(), ep_events[i].data.fd), myList.end());
                    close(ep_events[i].data.fd);
                    printf("closed client : %d \n", ep_events[i].data.fd);
                }
                else if(std::string(buf).find("[user]") != string::npos)
                {
                    // only user's request contain [user]
                    // todo sanity check
                    int id = get_id(buf);
                    //todo: consistent hashing 
                    int fd = pick_client_hash(ep_events[i].data.fd, std::to_string(id));
                    std::string new_buf = std::to_string(ep_events[i].data.fd) + ":" + std::string(buf); 
#ifdef DEBUG
std::cout << "forwarding message to client: " << fd << std::endl;
std::cout << new_buf << std::endl;
#endif
                    auto start = std::chrono::high_resolution_clock::now();
                    send(fd, new_buf.c_str(), new_buf.size(), 0);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> duration = end - start;
                    std::cout <<"forward message time: " << duration.count() << std::endl;
                }
		        else
                {  
#ifdef DEBUG
                    std::cout << "len: " << str_len << std::endl;
                    std::cout << "sending message to user "<< buf << std::endl;  
#endif 
                    // todo add map relationship between cleint and user
                    std::vector<std::string> seps = split(buf, ':');
#ifdef DEBUG
                    std::cout << seps[0] << ":" << seps[1] << std::endl;
#endif

                    auto start = std::chrono::high_resolution_clock::now();
                    send(std::stoi(seps[0]), seps[1].c_str(), std::strlen(seps[1].c_str()), 0);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> duration = end - start;
                    std::cout <<"sending message to user time: " << duration.count() << std::endl;
                   
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);

    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
