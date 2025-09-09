/**
 * @file EventLoop.h
 * @brief 事件循环，负责等待 I/O 事件，并把事件分发给合适的处理器。不直接做 I/O
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include "EventLoop.h"
#include <stdexcept>
#include <unistd.h>
#include <iostream>

EventLoop::EventLoop() {
    epollFd = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd == -1)
        throw std::runtime_error("epoll_create1 failed");
}

EventLoop::~EventLoop() {
    if (epollFd != -1)
        close(epollFd);
}

void EventLoop::register_connection(std::shared_ptr<Connection> connection) {
    // 注册到 epoll
    int fd = connection->get_fd();
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
    // 维护 connections
    connections[fd] = std::move(connection);
}

void EventLoop::deregister_connection(int fd) {
    // 从 epoll 中删除
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
    // 从 connections 中删除
    connections.erase(fd); // 哈希表删除一个没有的元素也是安全的
}

void EventLoop::loop() {
    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int n = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            auto it = connections.find(fd);
            if (it == connections.end()) continue;
            Connection& conn = *it->second;


            if (conn.is_listen_fd()) {
                conn.handle_accept(*this);
            }
            else if (events[i].events & EPOLLIN) {
                conn.handle_read(*this);
            }
            else if (events[i].events & EPOLLOUT) {
                conn.handle_write(*this);
            }
        }
    }
}
