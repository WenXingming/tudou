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


 /// @brief 连接建立事件。创建连接套接字: accept()
 /// @param fdListen 
 /// @param fds 
 /// @param maxfd 
void connect(int fdListen, pollfd* fds, int& maxfd) {
    printf("fdListen: %d is set.\n", fdListen);

    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int fdClient = accept(fdListen, (sockaddr*)&clientAddr, &len); // 阻塞。其实不会阻塞，因为 if 已经说明了有连接请求
    if (fdClient == -1) {
        perror("accept failed!\n");
        return;
    }

    printf("fdClient: %d is accepted.\n", fdClient);

    maxfd = std::max(maxfd, fdClient + 1);
    fds[fdClient].fd = fdClient;
    fds[fdClient].events = POLLIN;
}


/// @brief 传输、处理数据: recv()、close()、send()。注意是一次数据传输，所以不用 while 循环
/// @param fd 
/// @param fds 
/// @attention 这里不够细粒度，同时包含接受、发送数据，后续学习 reactor 会将其更加详细分类
void data_process(int fd, pollfd* fds) {
    char buffer[128] = { 0 };
    int length = recv(fd, buffer, 128, 0); // 阻塞函数
    if (length == -1) {
        perror("recv error!\n");
    }

    if (length == 0) {
        close(fd);
        printf("close fdClient: %d\n", fd);
        fds[fd].fd = -1;
        fds[fd].events = 0;
        return;
    }

    printf("buffer: %s", buffer); // 处理数据

    int retSend = send(fd, buffer, length, 0);
    if (retSend == -1) {
        perror("send error!\n");
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


    /// DO: pool 需要的数据结构初始化。不像 select 采用位数组，poll 采用结构体数组
    pollfd fds[1024] = { 0 };
    fds[fdListen].fd = fdListen;
    fds[fdListen].events = POLLIN;
    int maxFd = fdListen + 1;

    // 一直处理 IO 多路复用事件
    while (true) {
        int nready = poll(fds, maxFd, -1); // 阻塞。maxfd 非强制使用，可以写 1024，只是为了遍历少一些。最优是 vector + nfds
        if (nready == -1) {
            perror("poll error!\n");
            continue;
        }
        if (nready == 0) continue;

        // 遍历找位数组置 1（就绪即有 IO 事件） 的 fd
        for (int i = 0; i < maxFd; ++i) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == fdListen) {
                    connect(fdListen, fds, maxFd);
                }
                else {
                    data_process(i, fds);
                }
            }
            else;
        }
    }

    getchar();
}