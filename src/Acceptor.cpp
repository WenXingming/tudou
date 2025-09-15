#include "Acceptor.h"
#include "EventLoop.h"
#include "Channel.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <cassert>

#include "../base/Timestamp.h"
#include "../base/Log.h"


Acceptor::Acceptor(EventLoop* _loop, const InetAddress& _listenAddr) // 构造函数里访问 this 需要小心一些：有些成员变量没有在初始化列表里，是默认初始化
    : loop(_loop)
    , listenAddr(_listenAddr)
    , newConnectionCallback(nullptr) {

    // 初始化 this->listenFd
    this->create_fd();
    this->bind_address();
    this->listen_start();

    // 初始化 channel. 也可以放在初始化列表里，但注意初始化顺序（依赖 listenFd）
    // 注意：创建 channel 后需要设置 intesting event 和 订阅（发生事件后的回调函数）；并注册到 poller
    this->channel.reset(new Channel(this->loop, this->listenFd));
    this->channel->enable_reading();
    this->channel->subscribe_on_read(std::bind(&Acceptor::read_callback, this));
    this->loop->update_channel(channel.get());
}

Acceptor::~Acceptor() {
    ::close(this->listenFd); // listenFd 生命期应该由 Acceptor 管理（创建和销毁）
}



void Acceptor::create_fd() {
    this->listenFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); // 直接非阻塞
    if (this->listenFd == -1) {
        LOG::LOG_FATAL("Acceptor::create_fd(). create listenFd failed, errno is: %d.", errno);
    }
}

void Acceptor::bind_address() {
    sockaddr_in address = this->listenAddr.get_sockaddr();
    int bindRet = ::bind(this->listenFd, (sockaddr*)&address, sizeof(address));
    if (bindRet == -1) {
        LOG::LOG_FATAL("Acceptor::bind_address(). create listenFd failed, errno is: %d.", errno);
    }
}

void Acceptor::listen_start() {
    int listenRet = ::listen(this->listenFd, SOMAXCONN);
    if (listenRet == -1) {
        LOG::LOG_FATAL("Acceptor::listen_start(). listen failed, errno is: %d.", errno);
    }
}

void Acceptor::read_callback() {
    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int connFd = ::accept(this->listenFd, (sockaddr*)&clientAddr, &len);
    if (connFd >= 0) {
        LOG::LOG_DEBUG("Acceptor::handle_read(). connectFd %d is accepted.");

        publish_new_connection(connFd);
    }
    else {
        LOG::LOG_ERROR("Acceptor::handle_read(). accept error, errno: %d", errno);
    }
}

void Acceptor::subscribe_new_connection(std::function<void(int)> cb) {
    this->newConnectionCallback = std::move(cb);
}

void Acceptor::publish_new_connection(int connFd) {
    if (newConnectionCallback)
        newConnectionCallback(connFd);
    else {
        LOG::LOG_ERROR("Acceptor::publish_new_connection(). No newConnectionCallback setted.");
        ::close(connFd);
    }
}