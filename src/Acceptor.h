/**
 * @file Acceptor.h
 * @brief 监听新连接的接入器（封装 listenFd 及持有其 Channel），在有连接到来时接受并上报给上层。
 * @author wenxingming
 * @project: https://github.com/WenXingming/tudou
 * @details
 *
 * 职责：
 * - 封装监听套接字的创建、绑定与监听（create_fd/bind_address/listen_start）。
 * - 将 listenFd 注册到所属 EventLoop 的 Channel 上，监听可读事件（新连接到来）。
 * - 在可读事件触发时执行 read_callback，循环 accept 并生成 connFd，然后通过回调发布给上层（TcpServer）。
 * - 对外提供 subscribe_on_connect()，用于上层设置“新连接回调”，实现去耦。
 *
 * 线程模型与约定：
 * - 与所属 EventLoop 线程绑定，非线程安全方法需在该线程调用。
 * - 仅保存上层回调，不持有上层对象，避免环形依赖。
 *
 * 生命周期与所有权：
 * - 持有 Channel 的唯一所有权（std::unique_ptr），析构时自动解除注册与释放。
 * - 负责 listenFd 的管理；对已接受的 connFd 仅发布，不负责其后续生命周期（由上层 TcpConnection 管理）。
 *
 * 错误处理：
 * - accept 失败时记录日志并进行必要的失败分支处理（如 EAGAIN、资源耗尽等）。
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

    std::function<void(int)> connectCallback; // 单向依赖。保存上层 (TcpServer) 回调函数，上层拥有时初始化（订阅）

public:
    Acceptor(EventLoop* _loop, const InetAddress& _listenAddr, std::function<void(int)> _connectCallback);
    ~Acceptor();

    // 设置回调，上层 TcpServer 调用，类似于订阅者订阅话题。因为没有 master 注册中心，所以直接本地类保存回调函数
    void subscribe_on_connect(std::function<void(int)> cb);

private:
    void read_callback(); // channel 的回调处理函数。Acceptor 只需要处理连接事件

    void create_fd();
    void bind_address();
    void start_listen();

    void publish_on_connect(int connFd);
};
