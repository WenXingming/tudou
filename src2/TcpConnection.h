/**
 * @file TcpConnection.h
 * @brief
 * @details
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <memory>
#include <functional>
#include <string>

class EventLoop;
class Channel;
class Buffer;
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    using MessageCallback = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)>;
    using CloseCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;

private:
    int connectFd;
    std::unique_ptr<Channel> channel;
    std::unique_ptr<Buffer> inputBuffer;
    std::unique_ptr<Buffer> outputBuffer;

    EventLoop* loop;

    MessageCallback messageCallback; // 业务层回调（类似于 ROS 发布话题）
    CloseCallback closeCallback; // 应该是 TcpServer 回调（类似于 ROS 发布话题）

private:
    // 处理 channel 的回调
    void handle_read();
    void handle_write();
    void handle_close();

public:
    TcpConnection(EventLoop* _loop, int _sockfd);
    ~TcpConnection();

    int get_fd() const { return this->connectFd; }
    void setMessageCallback(MessageCallback _cb) { this->messageCallback = std::move(_cb); }
    void setCloseCallback(CloseCallback _cb) { this->closeCallback = std::move(_cb); }


    
    void send(const std::string& msg);
    void shutdown();


};
