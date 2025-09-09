/**
 * @file main.cpp
 * @brief 测试
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <iostream>
#include <memory>
#include <cassert>
#include <netinet/in.h>
#include <sys/socket.h>

#include "EventLoop.h"
#include "EventHander.h"
#include "Connection.h"
#include "RegistrationCenter.h"

int main() {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenFd != -1);

    sockaddr_in addr{};
    int port = 8080;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int retBind = bind(listenFd, (sockaddr*)&addr, sizeof(addr));
    assert(retBind != -1);

    int retListen = listen(listenFd, SOMAXCONN);
    assert(retListen != -1);

    // 创建注册中心
    std::shared_ptr<RegistrationCenter> registrationCenter = std::make_shared<RegistrationCenter>();
    // 创建事件循环 loop
    EventLoop loop(registrationCenter);
    // 创建 listenConn 并注册到 loop
    EchoHandler handler; // 业务逻辑处理类
    auto listenConn = std::make_shared<Connection>(registrationCenter, listenFd, true);
    listenConn->set_accept_cb([&handler](std::shared_ptr<RegistrationCenter> registrationCenter, Connection& listenConn) {
        handler.accept_cb(registrationCenter, listenConn);
        });
    registrationCenter->register_connection(listenConn);
    // 开启事件循环
    loop.loop();
}
