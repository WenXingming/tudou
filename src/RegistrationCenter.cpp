/**
 * @file RegistrationCenter.h
 * @brief Reactor 登记中心。每一个连接都要登记到此处：
 * @details: 1. 包括注册 fd 到 epoll； 2. 维护 connections
 * @author wenxingming
 * @date 2025-09-09
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

#include "RegistrationCenter.h"
#include "Connection.h"

static const bool debug = true; // 控制打印 debug 信息

RegistrationCenter::RegistrationCenter() {
    epollFd = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd == -1)
        throw std::runtime_error("epoll_create1 failed");
}

RegistrationCenter::~RegistrationCenter() {
    if (epollFd != -1)
        close(epollFd);
}

auto RegistrationCenter::get_connections()
-> std::unordered_map<int, std::shared_ptr<Connection>>& {
    return connections;
}

void RegistrationCenter::register_connection(std::shared_ptr<Connection> connection) {
    // 注册到 epoll
    int fd = connection->get_fd();
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
    // 维护 connections
    connections[fd] = std::move(connection);
}

void RegistrationCenter::deregister_connection(int fd) {
    // 从 epoll 中注销
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
    // 从 connections 中删除
    connections.erase(fd); // 哈希表删除一个没有的元素也是安全的
}
