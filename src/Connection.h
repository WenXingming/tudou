/**
 * @file Connection.h
 * @brief 事件源抽象 + 回调（Handler 风格）
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <vector>
#include <unistd.h>
#include <functional>
#include <fcntl.h>
#include <cstdio>

 /// @brief 连接抽象。Connection = fd + buffer + 回调函数
class EventLoop;
class Connection {
    int fd;
    std::vector<char> readBuffer;
    std::vector<char> writeBuffer;
    bool isListenFd;

    // 业务层的 3 个回调函数：每个连接可以自定义
    std::function<void(Connection&)> read_cb; // 业务层只需要 buffer 即可
    std::function<void(Connection&)> write_cb;
    std::function<void(EventLoop&, Connection&)> accept_cb; // 连接事件需要 loop（新 clientConn 注册到其中） 和 listenConn


public:
    explicit Connection(int _fd, bool _isListenFd = false);
    ~Connection();

    int get_fd() const { return fd; }
    bool is_listen_fd() const { return isListenFd; }
    std::vector<char>& get_read_buffer() { return readBuffer; }
    std::vector<char>& get_write_buffer() { return writeBuffer; }

    // 设置业务层的回调函数
    void set_read_cb(std::function<void(Connection&)> cb) { read_cb = std::move(cb); }
    void set_write_cb(std::function<void(Connection&)> cb) { write_cb = std::move(cb); }
    // void set_accept_cb(std::function<void(Connection&)> cb) { accept_cb = std::move(cb); }
    void set_accept_cb(std::function<void(EventLoop& loop, Connection&)> cb) { accept_cb = std::move(cb); }


    // 提供给 EventLoop 的 3 个回调函数（由 EventLoop 事件循环，触发事件后调用）
    void handle_read(EventLoop& loop);
    void handle_write(EventLoop& loop);
    // void handle_accept();
    void handle_accept(EventLoop& loop);

private:
    // 和底层 fd 通信：buffer <==> fd
    ssize_t read_from_fd();
    ssize_t write_to_fd();
};
