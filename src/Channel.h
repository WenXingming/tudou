/**
 * @file Channel.h
 * @brief 理解为事件的抽象，包含 event、revent、回调函数。可以用于事件发生后的回调处理
 * @author wenxingming
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <functional>
#include <cstdint>
#include <sys/epoll.h>
#include <memory>
#include "../base/NonCopyable.h"

class Timestamp;
class EventLoop;
class Channel : public NonCopyable {
private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop;    // 改变 channel 的 event 后，需要更新 channels 注册中心（在 poller），依赖 loop 完成（依赖注入设计）

    int fd;             // 并不持有，无需负责 close
    uint32_t event;     // interesting events
    uint32_t revent;    // received events types of poller, channel 调用 publish_event 时也根据 revent 进行事件分发回调

    std::function<void()> readCallback; // 根据 revents 调用事件的回调函数。没有 master，所以存放在这里
    std::function<void()> writeCallback;
    std::function<void()> closeCallback;
    std::function<void()> errorCallback;

private:
    void publish_read(); // 事件发生了就需要 publish。没有 master 注册中心，所以发布时直接本地自己触发回调 callback
    void publish_write();
    void publish_close();
    void publish_error();

    void update(); // 当改变 channel 的 event 后，需要在 poller 里面更改（更新） channel。何时被调用：调用 enable_reading 等函数改变 event 后

    void publish_event_with_guard(Timestamp receiveTime); // 根据事件调用回调函数。何时被调用：被 publish_event() 调用

public:
    explicit Channel(EventLoop* loop, int fd);
    ~Channel();

    void publish_event(Timestamp receiveTime); // 核心函数，事件发生后进行回调。何时被调用：EventLoop 事件循环中被调用（先通过 poller 返回活动 channels）


    void subscribe_on_read(std::function<void()> cb); // 没有办法，没有 master 注册中心，只能把订阅函数 callback 存在本类里，所以也需要提供公开接口
    void subscribe_on_write(std::function<void()> cb);
    void subscribe_on_close(std::function<void()> cb);
    void subscribe_on_error(std::function<void()> cb);

public:
    int get_fd() const { return fd; }
    uint32_t get_event() const { return event; }

    void enable_reading(); // 设置 fd 感兴趣的事件
    void disable_reading();
    void enable_writing();
    void disable_writing();
    void disable_all();

    void set_revent(uint32_t _revent) { revent = _revent; } // poller 监听到事件后设置此。handle 也根据此（发生的不同事件）执行不同的回调函数
};
