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


 /// @brief 连接建立事件。创建连接套接字: accept()
 /// @param fdListen 
 /// @param maxfd 
 /// @param reads 
 /// @return fdClient 
int connect_event(int fdListen, int& maxFd, fd_set& reads) {
    printf("fdListen: %d is set.\n", fdListen);

    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int fdClient = accept(fdListen, (sockaddr*)&clientAddr, &len); // 阻塞。其实不会阻塞，因为 if 已经说明了有连接请求
    if (fdClient == -1) {
        perror("accept failed!\n");
        return -1;
    }

    printf("fdClient: %d is accepted.\n", fdClient);

    // 维护 select 数据结构：maxFd、reads
    maxFd = std::max(maxFd, fdClient + 1);
    FD_SET(fdClient, &reads);

    return fdClient;
}

/// @brief 传输、处理数据: recv()、close()、send()。注意是一次数据传输，所以不用 while 循环
/// @param fdClient 
/// @param reads 
/// @attention 这里不够细粒度，同时包含接受、发送数据，后续学习 reactor 会将其更加详细分类
void data_process(int fdClient, fd_set& reads) {
    char buffer[1024] = { 0 };
    int length = recv(fdClient, buffer, 1024, 0); // 阻塞函数
    if (length == -1) {
        perror("recv error!\n");
        return;
    }

    if (length == 0) {
        close(fdClient);
        printf("close fdClient: %d\n", fdClient);
        FD_CLR(fdClient, &reads); // 维护 fd_set，socket 和 fd_set 一一对应
        return;
    }

    printf("buffer: %s", buffer); // 处理数据。计算密集型任务，可扔进线程池处理

    int retSend = send(fdClient, buffer, length, 0);
    if (retSend == -1) {
        perror("send error!\n");
        return;
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


    /// DO: select 需要的数据结构初始化。select(maxfd, reads, wset, eset, timeout)
    int maxFd = fdListen + 1;   // 我习惯用左闭右开
    fd_set reads;               // 位数组实现可读 fd 集合
    FD_ZERO(&reads);
    FD_SET(fdListen, &reads);   // 看源码，某一位置 1，构造一个数 | 运算

    // 一直处理 IO 多路复用事件
    while (true) {
        fd_set cpyReads = reads; // 必须 copy！
        int nready = select(maxFd, &cpyReads, NULL, NULL, NULL); // 阻塞。监听所有 reads
        if (nready == -1) {
            perror("select error!\n");
            return -1;
        }
        if (nready == 0) continue; // 主要是如果设置了 select 的 timeout，到时后阻塞状态会强行被停止，返回 0

        // 遍历找位数组置 1（就绪即有 IO 事件） 的 fd
        for (int i = 0; i < maxFd; ++i) {
            if (FD_ISSET(i, &cpyReads)) {
                if (i == fdListen) {
                    connect_event(fdListen, maxFd, reads); // listen 事件
                }
                else {
                    data_process(i, reads); // date process 事件（read 事件）
                }
            }
            else continue;
        }
    }

    getchar();
}