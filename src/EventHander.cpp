#include "EventHander.h"
#include <iostream>

#include "Connection.h"
#include "EventLoop.h"
#include <netinet/in.h>

static const bool debug = true;

void EventHandler::read_cb(Connection& conn) {
    if (debug) std::cout << "virtual function handle_read called.\n";
}

void EventHandler::write_cb(Connection& conn) {
    if (debug) std::cout << "virtual function handle_write called.\n";
}

void EventHandler::accept_cb(EventLoop& loop, Connection& conn) {
    if (debug) std::cout << "virtual function handle_accept called.\n";
}

void EchoHandler::read_cb(Connection& conn) {
    auto& buf = conn.get_read_buffer();
    std::string msg(buf.begin(), buf.end());
    std::cout << "recv: " << msg << std::endl;

    // 业务逻辑处理。这里是简单的 Echo 回写
    conn.get_write_buffer().insert(
        conn.get_write_buffer().end(),
        buf.begin(),
        buf.end()
    );
    buf.clear();
}

void EchoHandler::write_cb(Connection& conn) {
    std::cout << "send done on fd " << conn.get_fd() << std::endl;
}

void EchoHandler::accept_cb(EventLoop& loop, Connection& listenConn) {
    sockaddr_in clientAddr{};
    socklen_t len = sizeof(clientAddr);
    int clientFd = accept(listenConn.get_fd(), (sockaddr*)&clientAddr, &len);
    std::cout << "accept new client: " << clientFd << std::endl;

    auto clientConn = std::make_shared<Connection>(clientFd);
    clientConn->set_read_cb([this](Connection& conn) {
        this->read_cb(conn);
        });
    clientConn->set_write_cb([this](Connection& conn) {
        this->write_cb(conn);
        });
    loop.register_connection(clientConn);
}
