#include "EpollPoller.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "../base/Log.h"
#include "../base/Timestamp.h"
#include "Channel.h"
#include <assert.h>

// static const int kNew = -1; // channel index 初始化 -1 
// static const int kAdded = 1;
// static const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* _loop)
    : Poller(_loop) // 先调用基类构造函数
    , epollFd(epoll_create1(EPOLL_CLOEXEC))
    , eventList(initEventListSize) {

    if (epollFd < 0) {
        LOG::LOG_FATAL("EpollPoller::EpollPoller(). Failed to create epoll fd: %d", errno);
    }
}

EpollPoller::~EpollPoller() {
    if (epollFd > 0)
        close(epollFd);
    else LOG::LOG_FATAL("EpollPoller::~EpollPoller(). Failed to create epoll fd: %d", errno);
}

std::vector<Channel*> EpollPoller::poll(int timeoutMs) {
    std::vector<Channel*> activeChannels;

    LOG::LOG_DEBUG("EpollPoller::poll(). epoll is running... poller monitors channels's size is: %d", channels.size());
    int numReady = epoll_wait(epollFd, eventList.data() /* &eventList[0] */ /* &*eventList.begin() */, static_cast<int>(eventList.size()), timeoutMs);
    if (numReady > 0) {
        LOG::LOG_DEBUG("EpollPoller::poll(). epoll is running...  activeChannels's size is: %d", numReady);

        activeChannels.reserve(numReady); // 不是 resize()!
        activeChannels = fill_activate_channels(numReady);

        // eventList 自动扩容（可能有更多的事件发生, eventList 放不下）和缩减
        if (numReady == eventList.size()) {
            eventList.resize(eventList.size() * 2);
        }
        else if (numReady < eventList.size() * 0.1) {
            eventList.resize(eventList.size() * 0.5);
        }
    }

    return std::move(activeChannels);
}

std::vector<Channel*> EpollPoller::fill_activate_channels(int numReady) const {
    std::vector<Channel*> activeChannels;
    for (int i = 0; i < numReady; ++i) {
        const epoll_event& event = this->eventList[i];
        int fd = event.data.fd;
        auto revent = event.events;

        auto it = channels.find(fd);
        if (it != channels.end()) {
            Channel* channel = it->second;
            channel->set_revent(event.events);

            activeChannels.push_back(channel);
        }
        else LOG::LOG_ERROR("EpollPoller::fill_activate_channels(): epolled fd not in channels. epoll、channels not synchronous.");

        // auto channel = static_cast<Channel*>(event.data.ptr); // 一用这个就报错。之前加入 epoll 时设置了 ptr
        // channel->set_revent(revent);
        // activeChannels.push_back(channel);
    }
    return std::move(activeChannels);
}

void EpollPoller::update_channel(Channel* channel) {
    // if (channel->is_none_event()) { // 没有事件就应该删除吗？感觉不一定。可以就放在这里，由 channel 所属对象负责释放资源
    //     remove_channel(channel);
    //     return;
    // }

    int fd = channel->get_fd();
    auto event = channel->get_event();

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = event;

    // ev.data.ptr = static_cast<void*>(channel); // 报错
    // std::cout << channel->get_fd() << std::endl; // debug

    LOG::LOG_DEBUG("EpollPoller::update_channel(): fd is %d", fd); // debug
    if (channels.count(fd) == 0) {
        int epollCtlRet = epoll_ctl(epollFd, EPOLL_CTL_ADD, ev.data.fd, &ev);
        if (epollCtlRet == 0)
            channels[fd] = channel;
        else
            LOG::LOG_FATAL("EpollPoller::update_channel(): epoll_ctl add error: %d", errno);
    }
    else {
        int epollCtlRet = epoll_ctl(epollFd, EPOLL_CTL_MOD, ev.data.fd, &ev);
        if (epollCtlRet == 0) {
            assert(channels[fd] == channel);
            channels[fd] = channel; // 思考是否需要？按理说二者相等，上面也进行了断言
        }
        else
            LOG::LOG_FATAL("EpollPoller::update_channel(): epoll_ctl mod error: %d", errno);
    }
}

void EpollPoller::remove_channel(Channel* channel) {
    int fd = channel->get_fd();

    // epoll、channels should be synchronous.
    int epollCtlRet = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
    if (epollCtlRet == 0)
        channels.erase(fd);
    else
        LOG::LOG_ERROR("EpollPoller::remove_channel(): epoll_ctl del error: %d", errno);
}
