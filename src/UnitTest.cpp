/**
 * @file UnitTest.cpp
 * @brief 多路 IO 复用
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory>
#include <pthread.h>
#include <thread>
#include <poll.h>
#include <sys/poll.h>
#include <sys/epoll.h>


 /// @brief 连接建立事件。创建连接套接字: accept()
 /// @param fdListen 
 /// @param epfd 
void connect(int fdListen, int epfd) {
    printf("fdListen: %d is set.\n", fdListen);

    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int fdClient = accept(fdListen, (sockaddr*)&clientAddr, &len); // 阻塞。其实不会阻塞，因为 if 已经说明了有连接请求
    if (fdClient == -1) {
        perror("accept failed!");
        return;
    }

    printf("fdClient: %d is accepted.\n", fdClient);

    // 维护 epoll
    epoll_event event;
    event.events = EPOLLIN /* EPOLLIN | EPOLLET */; // 水平触发 LT, level trigger。边沿触发 ET, edge trigger，设置缓存较小试一试
    event.data.fd = fdClient;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fdClient, &event);
}


/// @brief 传输、处理数据: recv()、close()、send()。注意是一次数据传输，所以不用 while 循环
/// @param clientfd 
/// @param epfd 
/// @attention 这里不够细粒度，同时包含接受、发送数据，后续学习 reactor 会将其更加详细分类
void data_process(int clientfd, int epfd) {
    char buffer[1024] = { 0 };
    int length = recv(clientfd, buffer, 1024, 0); // 阻塞函数

    if (length == -1) {
        perror("recv error!");
    }

    if (length == 0) {
        printf("close clientfd.\n");
        close(clientfd);
        // 维护 epoll
        epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, NULL);
        return;
    }

    printf("buffer: %s", buffer); // 处理数据

    int retSend = send(clientfd, buffer, length, 0);
    if (retSend == -1) {
        perror("send error!");
    }
}


int main(int argc, char* argv[]) {
    // 创建监听套接字: socket()。
    int fdListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(fdListen != -1);

    // 绑定监听 ip + port: bind()
    const char* ip = "127.0.0.1";
    const char* port = "2048";

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到所有网络接口（包括虚拟回环接口和物理网络接口）
    if (argc == 2)
        serverAddr.sin_port = htons(atoi(argv[1]));
    else
        serverAddr.sin_port = htons(atoi(port));

    int retBind = bind(fdListen, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (retBind == -1) {
        perror("bind failed!\n");
        return -1;
    }

    // 开始监听: listen()
    int retListen = listen(fdListen, 10);
    if (retListen == -1) {
        perror("listen failed!\n");
        getchar();
        return -1;
    }


    /// DO: epoll 初始化
    int epfd = epoll_create(1); // 现在参数已经无用，> 0 即可（现使用链表无限扩展）。epfd 占用了 4 号 fd，所以 fdClient 从 5开始
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fdListen;
    epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev);

    // 一直处理 IO 多路复用事件
    epoll_event eventList[1024] = { 0 };
    while (true) {
        int nready = epoll_wait(epfd, eventList, 1024, -1);

        for (int i = 0; i < nready; ++i) {
            int fd = eventList[i].data.fd;
            if (eventList[i].events & EPOLLIN) {
                if (fd == fdListen) {
                    connect(fd, epfd);
                }
                else {
                    data_process(fd, epfd);
                }
            }
        }
    }

    getchar();
}