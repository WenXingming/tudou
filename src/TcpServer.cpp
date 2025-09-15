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

    acceptor->subscribe_new_connection(
        // 绑定 this 参数会有风险吗？此时构造函数还未执行完，就使用了 this 指针。其实也没事，只是绑定，没有通过 this 访问类成员（还未构造完不可访问，否则有风险）
        // 可以使用 lambda
        std::bind(&TcpServer::new_connection_callback, this, std::placeholders::_1)
    );
}

TcpServer::~TcpServer() {
    this->connections.clear();
    /* this->loop = nullptr;
    this->messageCallback = nullptr; */
}

void TcpServer::start() {
    // EventLoop 开始循环时，Acceptor 会自动监听
}



void TcpServer::new_connection_callback(int connFd) {
    LOG::LOG_DEBUG("TcpServer::new_connection(). new connection created. fd is: %d", connFd);

    // 初始化 conn
    auto conn = std::make_shared<TcpConnection>(loop, connFd);
    conn->subscribe_message(this->messageCallback); // 设置业务层回调函数。callback 是由业务传入的，TcpServer 并不实现 callback 只是做中间者
    conn->subscribe_close(std::bind(&TcpServer::close_callback, this, std::placeholders::_1));

    connections[connFd] = conn;
}

void TcpServer::close_callback(const std::shared_ptr<TcpConnection>& conn) {
    this->remove_connection(conn);
}

void TcpServer::remove_connection(const std::shared_ptr<TcpConnection>& conn) {
    int fd = conn->get_fd();
    connections.erase(fd);
}

void TcpServer::subscribe_message(MessageCallback cb) {
    this->messageCallback = cb;
}
