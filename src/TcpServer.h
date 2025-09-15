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
    using MessageCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;

private:
    std::unique_ptr<Acceptor> acceptor;
    std::map<int, std::shared_ptr<TcpConnection>> connections; // 生命期模糊，用户也可以持有。所以用 shared_ptr

    EventLoop* loop;

    MessageCallback messageCallback;


private:
    void new_connection_callback(int sockfd); // Acceptor 的回调处理函数
    void close_callback(const std::shared_ptr<TcpConnection>& conn); // TcpConnection 的回调函数
    void remove_connection(const std::shared_ptr<TcpConnection>& conn);

public:
    TcpServer(EventLoop* _loop, const InetAddress& _listenAddr);
    ~TcpServer();

    void start();

    void subscribe_message(MessageCallback cb); // TcpConnection 发布。不是 TcpServer 发布，Server 只是作为消息传递中间商
};
