/**
 * @file Acceptor.h
 * @brief Acceptor 负责监听端口（封装了 listenFd）
 * @details 当有新连接到来时 channel 被触发，调用当前回调 handleRead。当前回调创建 fd，并调用 TcpServer 回调（创建 TcpConnection）
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <functional>
#include <memory>

#include "../base/InetAddress.h"
#include "../base/NonCopyable.h"

class EventLoop;
class Channel;
class InetAddress;
class Acceptor : public NonCopyable {
private:
    EventLoop* loop; // channel 应该也要注册进 poller

    int listenFd;
    InetAddress listenAddr;
    std::unique_ptr<Channel> channel;

    std::function<void(int)> newConnectionCallback; // 单向依赖。保存上层 (TcpServer) 回调函数，上层拥有时初始化（订阅）

    
private:
    void read_callback(); // channel 的回调处理函数。Acceptor 只需要处理连接事件

    void create_fd();
    void bind_address();
    void listen_start();

    void publish_new_connection(int connFd); // 发布给 TcpServer：新连接来了。消息格式是 int

public:
    Acceptor(EventLoop* _loop, const InetAddress& _listenAddr);
    ~Acceptor();

    void subscribe_new_connection(std::function<void(int)> cb); // 设置回调。上层 TcpServer 调用，类似于订阅者订阅话题

};
