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
    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int fdClient = accept(fdListen, (sockaddr*)&clientAddr, &len); // 阻塞函数
    if (fdClient == -1) {
        perror("accept failed!\n");
        return -1;
    }

    // 我们希望一直接收、处理数据: recv()、close()、send()
    while (true) {
        std::shared_ptr<char> bufferPtr(new char[1024], [](char* p) {
            delete[] p;
            }); // 智能指针管理数组，自定义删除器
        int length = recv(fdClient, bufferPtr.get(), 1024, 0); // 阻塞函数
        if (length == -1) {
            perror("recv error!\n");
        }

        // 主动断开方会进入 time_wait... 短时间重新启动：bind failed! Address already in use。所以服务器尽量被动断开连接
        // length == 0 代表对方调用了 close()
        // 接收到 FIN，发送 ACK。根据状态机，服务器进入 CLOSE_WAIT 状态，但可以继续发送数据；服务端调用 close() 即可进入下个状态
        if (length == 0) {
            close(fdClient);
            printf("close fd: %d\n", fdClient);
            break;
        }

        printf("fdListen: %d, fdClient: %d, length: %d\n", fdListen, fdClient, length);
        printf("buffer: %s", bufferPtr.get()); // 处理数据，可扔给线程池处理（IO任务和计算密集型任务解耦）

        int retSend = send(fdClient, bufferPtr.get(), length, 0); // 发送数据，这是一个简单的回声服务器
        if (retSend == -1) {
            perror("send error!\n");
        }
    }

    getchar();
}