/**
 * @file Channel.cpp
 * @brief 理解为通道，用于事件回调处理
 * @author wenxingming
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include "Channel.h"
#include "EventLoop.h"
#include "../base/Timestamp.h"
#include "../base/Log.h"
#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* _loop, int _fd)
    : fd(_fd)
    , event(0)
    , revent(0)
    , loop(_loop)
    // , index(-1)
    /* tiePtr(std::shared_ptr<void>(nullptr)), */ {}

Channel::~Channel() {}

void Channel::handle_event(Timestamp receiveTime) {
    if (!tiePtr.expired()) { // tiePtr.use_count() != 0
        auto guard = tiePtr.lock();
        if (guard) {
            handle_event_with_guard(receiveTime);
        }
    }
    else {
        handle_event_with_guard(receiveTime);
    }
}

void Channel::handle_event_with_guard(Timestamp receiveTime) {
    LOG::LOG_INFO("poller find event, then channel handle events: %d", revent);

    if ((revent & EPOLLHUP) && !(revent & EPOLLIN)) {
        if (closeCallback) closeCallback();
        return;
    }
    if (revent & (EPOLLERR)) {
        if (errorCallback) errorCallback();
        return;
    }
    if (revent & (EPOLLIN | EPOLLPRI)) {
        if (readCallback) readCallback(receiveTime);
    }
    if (revent & EPOLLOUT) {
        if (writeCallback) writeCallback();
    }
}



/// @brief 在 poller 中把当前 channel 删除
void Channel::remove() {
    loop->remove_channel(this);
}

void Channel::update() {
    loop->update_channel(this);
}

void Channel::enable_reading() {
    event |= kReadEvent;
    update();
}

void Channel::disable_reading() {
    event &= ~kReadEvent;
    update();
}

void Channel::enable_writing() {
    event |= kWriteEvent;
    update();
}

void Channel::disable_writing() {
    event &= ~kWriteEvent;
    update();
}

void Channel::disable_all() {
    event = kNoneEvent;
    update();
}

/// @brief 弱智能指针监听某个 shared_ptr
/// @details 何时被调用？
void Channel::tie(const std::shared_ptr<void>& obj) {
    tiePtr = obj;
    // isTied = true;
}
