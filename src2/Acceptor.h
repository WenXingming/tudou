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
    int listenFd;
    InetAddress listenAddr;
    std::unique_ptr<Channel> channel;
    EventLoop* loop; // channel 应该也要注册进 poller。在 muduo 的设计里, poller 并不拥有 channel

    std::function<void(int)> newConnectionCallback; // 尽量单向依赖。上层(TcpServer)回调，上层拥有时初始化

private:
    void handle_read();
    void create_fd();
    void bind_address();
    void start_listen();

public:
    Acceptor(EventLoop* _loop, const InetAddress& _listenAddr);
    ~Acceptor();

    // 设置回调。上层 TcpServer 调用，类似于订阅者订阅话题
    void set_new_connection_callback(std::function<void(int)> cb) { newConnectionCallback = std::move(cb); }
};
