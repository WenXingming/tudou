/**
 * @file EventLoop.h
 * @brief 事件循环类，负责等待 I/O 事件，并进行相应的回调函数处理。不直接做 I/O
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include "EventLoop.h"
#include "RegistrationCenter.h"
#include "Connection.h"
#include <sys/epoll.h>

EventLoop::EventLoop(std::shared_ptr<RegistrationCenter> _registrationCenter)
    : registrationCenter(_registrationCenter) {}

void EventLoop::loop() {
    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int epollFd = registrationCenter->get_epoll_fd();
        auto& connections = registrationCenter->get_connections();

        int n = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            auto it = connections.find(fd);
            if (it == connections.end()) continue;
            Connection& conn = *it->second;

            if (conn.is_listen_fd()) {
                conn.handle_accept();
            }
            else if (events[i].events & EPOLLIN) {
                conn.handle_read();
            }
            else if (events[i].events & EPOLLOUT) {
                conn.handle_write();
            }
        }
    }
}
