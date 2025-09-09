/**
 * @file main.cpp
 * @brief 测试
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <cassert>
#include "EventLoop.h"
#include "EventHander.h"

int main() {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenFd != -1);

    sockaddr_in addr{};
    int port = 2048;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int retBind = bind(listenFd, (sockaddr*)&addr, sizeof(addr));
    assert(retBind != -1);

    int retListen = listen(listenFd, SOMAXCONN);
    assert(retListen != -1);


    EventLoop loop;
    EchoHandler handler;

    auto listenConn = std::make_shared<Connection>(listenFd, true);
    listenConn->set_accept_cb([&handler](EventLoop& loop, Connection& listenConn) {
        handler.accept_cb(loop, listenConn);
        });
    loop.register_connection(listenConn);

    loop.loop();
}
