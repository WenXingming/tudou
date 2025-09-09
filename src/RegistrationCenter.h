/**
 * @file RegistrationCenter.h
 * @brief Reactor 登记中心。每一个连接都要登记到此处：
 * @details: 1. 包括注册 fd 到 epoll； 2. 维护 connections
 * @author wenxingming
 * @date 2025-09-09
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <unordered_map>
#include <memory>

 // 头文件只使用前向声明、源文件包含头文件，是一个好习惯吗？
class Connection;

/// @brief 每个线程只实例化一个登记中心
class RegistrationCenter {
private:
    int epollFd;
    std::unordered_map<int, std::shared_ptr<Connection>> connections; // 用 shared_ptr close(fd)，确保（最后一个）析构时调用 close()。缺点是会不会某个函数（全局变量）一直持有 shared_ptr 导致内存泄漏？这里不像协程函数可以 yield()，应该不会

public:
    RegistrationCenter();
    ~RegistrationCenter();

    int get_epoll_fd() { return epollFd; }
    auto get_connections()
        -> std::unordered_map<int, std::shared_ptr<Connection>>&;

    /// @brief 注册 connection 到 Reactor(EventLoop)
    /// @param connection 
    void register_connection(std::shared_ptr<Connection> connection);
    void deregister_connection(int fd);
};