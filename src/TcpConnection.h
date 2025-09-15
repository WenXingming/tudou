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
    using MessageCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;
    using CloseCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;

private:
    EventLoop* loop;

    int connectFd;
    std::unique_ptr<Channel> channel;
    std::unique_ptr<Buffer> readBuffer;
    std::unique_ptr<Buffer> writeBuffer;

    MessageCallback messageCallback; // 业务层回调（类似于 ROS 发布话题）
    CloseCallback closeCallback; // 应该是 TcpServer 回调（类似于 ROS 发布话题）

private:
    // 处理 channel 事件的回调函数。没有设置 error call_back()
    void read_callback();
    void write_callback();
    void close_callback();
    void error_callback();

    void publish_message();
    void publish_close();

public:
    TcpConnection(EventLoop* _loop, int _sockfd);
    ~TcpConnection();

    int get_fd() const { return this->connectFd; }
    Buffer* get_input_buffer();
    Buffer* get_output_buffer();

    // 由于没有 master 注册中心。所以发布的类需要同时处理订阅...，本类就是 master
    // TcpConnection <==> 业务层
    void subscribe_message(MessageCallback _cb);
    // TcpConnection <==> TcpServer
    void subscribe_close(CloseCallback _cb);


    void send(const std::string& msg);
    std::string recv();

    /* void shutdown(); */
};
