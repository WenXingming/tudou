/**
 * @file EventLoop.cpp
 * @brief 事件循环类，负责事件循环
 * @author wenxingming
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include "EventLoop.h"
#include <iostream>
#include <assert.h>
#include <thread>
#include <unistd.h>      // 定义 syscall, pid_t
#include <sys/eventfd.h>
#include <sys/syscall.h> // 定义 SYS_gettid

#include "../base/Timestamp.h"
#include "../base/Log.h"
#include "Poller.h"
#include "Channel.h"

EventLoop::EventLoop()
    : poller(Poller::new_default_poller(this))
    , pollTimeoutMs(5000) {}


EventLoop::~EventLoop() {}


void EventLoop::loop() {
    LOG::LOG_INFO("EventLoop start looping...");
    while (true) {
        LOG::LOG_INFO("EventLoop is looping...");

        // 使用 poller 监听发生事件的 channels
        std::vector<Channel*> activeChannels = this->poller->poll(this->pollTimeoutMs);

        // 通知 channel 处理回调
        for (auto channel : activeChannels) {
            channel->publish_event(Timestamp::now()); // 也监听了 wakeupFd
        }
    }
    LOG::LOG_INFO("EventLoop stop looping.");
}


void EventLoop::update_channel(Channel* channel) {
    this->poller->update_channel(channel);
}

void EventLoop::remove_channel(Channel* channel) {
    this->poller->remove_channel(channel);
}
