/**
 * @file EventLoop.h
 * @brief 事件循环类，负责事件循环
 * @author wenxingming
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <memory>
#include "../base/NonCopyable.h"

class Poller;
class Channel;
class Timestamp;
class EventLoop : NonCopyable {
private:
    std::unique_ptr<Poller> poller; // 拥有 poller。智能指针，自动析构
    const int pollTimeoutMs = 5000; // poller 默认的超时时间

public:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop();

    void update_channel(Channel* channel);
    void remove_channel(Channel* channel);
};
