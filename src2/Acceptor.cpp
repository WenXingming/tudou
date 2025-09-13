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


Acceptor::Acceptor(EventLoop* _loop, const InetAddress& _listenAddr)
    : loop(_loop)
    , listenAddr(_listenAddr) {

    // 创建 listenFd、绑定地址端口、开始监听。发生错误则终止程序（LOG::LOG_FATAL）
    this->create_fd();
    this->bind_address();
    this->start_listen();

    // 创建 listenFd 相应的 channel 并注册到 poller。注意：创建 channel 后需要设置 intesting event 和 callback
    channel.reset(new Channel(this->loop, this->listenFd)); // 也可以放在初始化列表里，但注意初始化顺序（依赖 listenFd）
    channel->enable_reading();
    channel->set_read_callback(
        [this](Timestamp receivetime) { handle_read(); }
    );
    loop->update_channel(channel.get()); // 其实 enable_reading() 已经注册进了 poller，这里又注册到 poller 了一次（还是不要省，只要 channel 改变就 update。虽然理论上只有 event 改变需要调用）
}

Acceptor::~Acceptor() {
    ::close(listenFd); // listenFd 生命期应该由 Acceptor 管理（创建和销毁）
}

void Acceptor::create_fd() {
    this->listenFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); // 直接非阻塞
    if (listenFd == -1) {
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

void Acceptor::start_listen() {
    int listenRet = ::listen(this->listenFd, SOMAXCONN);
    if (listenRet == -1) {
        LOG::LOG_FATAL("Acceptor::start_listen(). listen failed, errno is: %d.", errno);
    }
}

void Acceptor::handle_read() {
    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int connfd = ::accept(this->listenFd, (sockaddr*)&clientAddr, &len);
    if (connfd >= 0) {
        LOG::LOG_DEBUG("Acceptor::handle_read(). connectFd %d is accepted.");
        if (this->newConnectionCallback) {
            this->newConnectionCallback(connfd); // 执行上层 TcpServer 的回调函数（类似于发布者发布话题），TcpServer 创建 TcpConnection
        }
        else {
            LOG::LOG_DEBUG("Acceptor::handle_read(). no set newConnectionCallback.");
            ::close(connfd);
        }
    }
    else {
        LOG::LOG_ERROR("Acceptor::handle_read(). accept error, errno: %d", errno);
    }
}
