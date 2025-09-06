#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    // 创建监听套接字: socket()。
    // fd 描述符依次增加，从 3 开始。因为 std::in、std::out、std::err 分别是 0、1、2
    // ls / dev / std * 查看 fd
    int fdListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdListen == -1) {   // assert(sockfdListen != -1);
        perror("sockfdListen create failed!\n");
        return -1;
    }

    // 绑定监听 ip + port: bind()
    // 使用的 ip 地址和 port 以结构体（sockaddr_in）的形式给出了定义
    const char* ip = "127.0.0.1"; // 虚拟回环接口。注意和物理网络接口（如eth0、wlan0，例如192.168.3.2）是各自独立的，无法通过 192.168.3.2:2048 访问
    const char* port = "2048";

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip); // 基于字符串的地址初始化，返回网络序（大端）。字节序：如果直接把主机 CPU 字节序的值放进sockaddr_in结构体，然后发到网络上，不同端的主机会读错ip、port（不同主机 CPU 大小端不同）
    if (argc == 2) serverAddr.sin_port = htons(atoi(argv[1]));
    else serverAddr.sin_port = htons(atoi(port));

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

    getchar();
}