/**
 * @file TcpServer.h
 * @brief TCP 服务器：管理 Acceptor 与 TcpConnection，会话创建、回调接线与连接生命周期管理。
 * @author
 * @project: https://github.com/WenXingming/tudou
 * @details
 *
 * 职责：
 * - 持有 Acceptor，监听并接受新连接，在回调中创建/接管 TcpConnection。
 * - 维护 fd->TcpConnection 的映射，负责连接增删及资源回收。
 * - 将业务层 MessageCallback 与 TcpConnection 的 message/close 回调进行接线与转发。
 *
 * 线程模型与约定：
 * - 与所属 EventLoop 线程绑定，非线程安全；所有对外方法应在该线程调用。
 *
 * 生命周期与所有权：
 * - 唯一拥有 Acceptor（std::unique_ptr）。
 * - 以 std::shared_ptr 管理 TcpConnection 的生命周期，允许业务层持有副本。
 * - 仅保存回调，不持有上层业务对象，避免环依赖。
 *
 * 运行流程：
 * - start(): 启动监听（Acceptor 注册读事件）。
 * - on_connect_callback(): 接受连接并创建 TcpConnection，注册读/写/关闭/错误回调。
 * - close_callback()/remove_connection(): 从映射中移除并清理。
 *
 * 错误处理：
 * - 接受新连接失败或资源不足时记录并忽略本次事件，保持服务可用。
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
    EventLoop* loop;

    std::unique_ptr<Acceptor> acceptor;
    std::map<int, std::shared_ptr<TcpConnection>> connections; // 生命期模糊，用户也可以持有。所以用 shared_ptr

    MessageCallback messageCallback;

private:
    void on_connect_callback(int sockfd); // Acceptor 的回调处理函数
    void close_callback(const std::shared_ptr<TcpConnection>& conn); // TcpConnection 的回调函数
    void remove_connection(const std::shared_ptr<TcpConnection>& conn);

public:
    TcpServer(EventLoop* _loop, const InetAddress& _listenAddr, TcpServer::MessageCallback _messageCallback);
    ~TcpServer();

    void start();

    // TcpConnection 发布。不是 TcpServer 发布，Server 只是作为消息传递中间商
    void subscribe_message(MessageCallback cb);
};
