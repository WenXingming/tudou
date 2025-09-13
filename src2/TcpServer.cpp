#include "TcpServer.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <functional>

#include "../base/InetAddress.h"
#include "../base/Log.h"

TcpServer::TcpServer(EventLoop* _loop, const InetAddress& _listenAddr)
    : acceptor(new Acceptor(_loop, _listenAddr))
    , connections{}
    , loop(_loop)
    , messageCallback(nullptr) {

    acceptor->set_new_connection_callback(
        // &TcpServer::new_connection // 成员函数有默认 this 参数
        // 绑定 this 参数会有风险吗？此时构造函数还未执行完，就使用了 this 指针。其实也没事，只是绑定，没有通过 this 访问类成员（还未构造完不可访问，否则有风险）
        std::bind(&TcpServer::new_connection, this, std::placeholders::_1)

        /*
        // 使用 lambda 简明方便
        [this](int _fd) {
            new_connection(_fd);
        }
        */
    );
}

TcpServer::~TcpServer() {
    this->connections.clear();
    // this->loop = nullptr;
    // this->messageCallback = nullptr;
}

void TcpServer::start() {
    // EventLoop 开始循环时，Acceptor 会自动监听
}

void TcpServer::new_connection(int connFd) {
    LOG::LOG_DEBUG("TcpServer::new_connection(). new connection created. fd is: %d", connFd);

    auto conn = std::make_shared<TcpConnection>(loop, connFd);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback([this](const std::shared_ptr<TcpConnection>& conn) {
        remove_connection(conn);
        });

    connections[connFd] = conn;
}

void TcpServer::remove_connection(const std::shared_ptr<TcpConnection>& conn) {
    int fd = conn->get_fd();
    connections.erase(fd);
}
