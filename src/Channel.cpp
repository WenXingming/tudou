/**
 * @file Channel.h
 * @brief Channel 用于把 IO 事件与回调绑定起来的抽象。
 * @author wenxingming
 * @project: https://github.com/WenXingming/tudou
 *
 */

#include "Channel.h"
#include "EventLoop.h"
#include "../base/Timestamp.h"
#include "../base/Log.h"
#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

// 构造函数。把 fd、event、revent、readCallback 等都设置为参数可能更好，避免使用时忘记设置（提醒使用者）
Channel::Channel(EventLoop* _loop, int _fd, uint32_t _event, uint32_t _revent,
    std::function<void()> _readCallback, std::function<void()> _writeCallback,
    std::function<void()> _closeCallback, std::function<void()> _errorCallback)
    : loop(_loop)
    , fd(_fd)
    , event(_event)
    , revent(_revent)
    , readCallback(std::move(_readCallback))
    , writeCallback(std::move(_writeCallback))
    , closeCallback(std::move(_closeCallback))
    , errorCallback(std::move(_errorCallback)) {

}

Channel::~Channel() {

}

void Channel::publish_events(Timestamp receiveTime) {
    publish_events_with_guard(receiveTime);
}

void Channel::publish_events_with_guard(Timestamp receiveTime) {
    LOG::LOG_DEBUG("poller find event, channel publish event: %d", revent);

    if ((revent & EPOLLHUP) && !(revent & EPOLLIN)) {
        this->publish_close();
        return;
    }
    if (revent & (EPOLLERR)) {
        this->publish_error();
        return;
    }
    if (revent & (EPOLLIN | EPOLLPRI)) {
        this->publish_read();
    }
    if (revent & EPOLLOUT) {
        this->publish_write();
    }
}

void Channel::enable_reading() {
    this->event |= Channel::kReadEvent;
    update();
}

void Channel::disable_reading() {
    this->event &= ~Channel::kReadEvent;
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

void Channel::set_revent(uint32_t _revent) {
    revent = _revent;
}

void Channel::update() {
    loop->update_channel(this);
}

void Channel::subscribe_on_read(std::function<void()> cb) {
    this->readCallback = std::move(cb);
}

void Channel::subscribe_on_write(std::function<void()> cb) {
    this->writeCallback = std::move(cb);
}

void Channel::subscribe_on_close(std::function<void()> cb) {
    this->closeCallback = std::move(cb);
}

void Channel::subscribe_on_error(std::function<void()> cb) {
    this->errorCallback = std::move(cb);
}

int Channel::get_fd() const {
    return fd;
}

uint32_t Channel::get_event() const {
    return event;
}

void Channel::publish_read() {
    if (this->readCallback) {
        this->readCallback();
    }
    else {
        LOG::LOG_ERROR("Channel::publish_read(). no readCallback.");
    }
}

void Channel::publish_write() {
    if (this->writeCallback) {
        this->writeCallback();
    }
    else {
        LOG::LOG_ERROR("Channel::publish_write(). no writeCallback.");
    }
}

void Channel::publish_close() {
    if (this->closeCallback) {
        this->closeCallback();
    }
    else {
        LOG::LOG_ERROR("Channel::publish_close(). no closeCallback.");
    }
}

void Channel::publish_error() {
    if (this->errorCallback) {
        this->errorCallback();
    }
    else {
        LOG::LOG_ERROR("Channel::publish_error(). no errorCallback.");
    }
}
