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
    : loop(_loop)
    , fd(_fd)
    , event(0)
    , revent(0)
    , readCallback(nullptr)
    , writeCallback(nullptr)
    , closeCallback(nullptr)
    , errorCallback(nullptr) {}

Channel::~Channel() {}

void Channel::publish_event(Timestamp receiveTime) {
    publish_event_with_guard(receiveTime);
}

void Channel::publish_event_with_guard(Timestamp receiveTime) {
    LOG::LOG_INFO("poller find event, channel publish event: %d", revent);

    if ((revent & EPOLLHUP) && !(revent & EPOLLIN)) {
        this->publish_close();
        return;
    }
    if (revent & (EPOLLERR)) {
        this->publish_error();
        return;
    }
    if (revent & (EPOLLIN | EPOLLPRI)) {
        // if (readCallback) readCallback(/* receiveTime */);
        this->publish_read();
    }
    if (revent & EPOLLOUT) {
        // if (writeCallback) writeCallback();
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

void Channel::update() {
    loop->update_channel(this);
}

void Channel::subscribe_on_read(std::function<void(/* Timestamp */)> cb) {
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

void Channel::publish_read() {
    if (this->readCallback)
        this->readCallback();
    else LOG::LOG_ERROR("Channel::publish_read(). no readCallback.");
}

void Channel::publish_write() {
    if (this->writeCallback)
        this->writeCallback();
    else LOG::LOG_ERROR("Channel::publish_write(). no writeCallback.");
}

void Channel::publish_close() {
    if (this->closeCallback)
        this->closeCallback();
    else LOG::LOG_ERROR("Channel::publish_close(). no closeCallback.");
}

void Channel::publish_error() {
    if (this->errorCallback)
        this->errorCallback();
    else LOG::LOG_ERROR("Channel::publish_error(). no errorCallback.");
}

