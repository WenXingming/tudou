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


// 传输、处理数据: recv()、close()、send()
void client_thread(int fdClient) {
    while (true) {
        char buffer[1024] = { 0 };
        int length = recv(fdClient, buffer, 1024, 0); // 阻塞函数
        if (length == -1) {
            perror("recv error!\n");
        }

        if (length == 0) {
            close(fdClient);
            printf("close fd: %d\n", fdClient);
            break;
        }

        printf("buffer: %s", buffer); // 处理数据。可扔给线程池处理（IO和计算密集型解耦）

        int retSend = send(fdClient, buffer, length, 0);
        if (retSend == -1) {
            perror("send error!\n");
        }
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

    // 创建连接套接字: accept()
    while (true) {
        sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);
        int fdClient = accept(fdListen, (sockaddr*)&clientAddr, &len); // 阻塞函数
        if (fdClient == -1) {
            perror("accept failed!\n");
            return -1;
        }

        // 每个连接套接字分配一个线程进行数据收发、处理
        std::thread t(client_thread, fdClient); // 异步
        t.detach(); // 每定义一个线程必须确定是 join() or detach()
    }

    getchar();
}