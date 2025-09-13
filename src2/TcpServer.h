/**
 * @file TcpServer.h
 * @brief 管理 Acceptor 和 TcpConnections
 * @details
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <map>
#include <memory>
#include <string>
#include <functional>

class EventLoop;
class Acceptor;
class TcpConnection;
class Buffer;
class InetAddress;
class TcpServer {
    using MessageCallback = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)>;

private:
    std::unique_ptr<Acceptor> acceptor;
    std::map<int, std::shared_ptr<TcpConnection>> connections; // 生命期模糊，用户也可以持有。所以用 shared_ptr

    EventLoop* loop;

    MessageCallback messageCallback;

private:
    void new_connection(int sockfd); // Acceptor 的回调函数

    void remove_connection(const std::shared_ptr<TcpConnection>& conn);

public:

    TcpServer(EventLoop* _loop, const InetAddress& _listenAddr);
    ~TcpServer();

    void start();

    void setMessageCallback(MessageCallback cb) { messageCallback = std::move(cb); }


};
