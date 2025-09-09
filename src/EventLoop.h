/**
 * @file EventLoop.h
 * @brief 事件循环，负责等待 I/O 事件，并进行相应的回调函数处理。不直接做 I/O
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include "Connection.h"

class EventLoop {
    int epollFd;
    std::unordered_map<int, std::shared_ptr<Connection>> connections; // 用 shared_ptr close(fd)，确保（最后一个）析构时调用 close()。缺点是会不会某个函数（全局变量）一直持有 shared_ptr 导致内存泄漏？这里不像协程函数可以 yield()，应该不会


public:
    explicit EventLoop();
    ~EventLoop();

    /// @brief 注册 connection 到 Reactor(EventLoop)
    /// @param connection 
    int get_epoll_fd() { return epollFd; }
    void register_connection(std::shared_ptr<Connection> connection);
    void deregister_connection(int fd);

    void loop();
};

