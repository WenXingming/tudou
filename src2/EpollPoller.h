/**
 * @file EpollPoller.h
 * @brief EpollPoller 封装 epoll, 作为多路事件分发器, 是 IO 服用核心模块 (epoll_create1 / epoll_ctl(add, mod, del) / epoll_wait)
 * @details 可以理解为一个注册中心（注册 epollfd、channel）
 * @author wenxingming
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include "Poller.h"
#include <vector>
#include <sys/epoll.h>

class EpollPoller : public Poller {
private:
    int epollFd;

    const int initEventListSize = 16;
    std::vector<epoll_event> eventList;

private:
    std::vector<Channel*> fill_activate_channels(int numEvents) const;
    void update(int operation, Channel* channel);

public:
    EpollPoller(EventLoop* _loop); // epoll_create()
    ~EpollPoller() override;

    /// @brief 返回 activeChannels。使用 epoll_wait()
    std::vector<Channel*> poll(int timeoutMs) override;

    /// @brief 维护注册中心 epollfd、channels。使用 epoll_ctl(), 包括 add、del、mod
    void update_channel(Channel* channel) override;
    void remove_channel(Channel* channel) override;

};