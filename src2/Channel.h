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
    /* using std::function<void()> = std::function<void()>;
    using std::function<void(Timestamp)> = std::function<void(Timestamp)>; */

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    int fd;             // 并不持有，无需负责 close
    uint32_t event;     // interesting events
    uint32_t revent;    // received events types of poller, channel 调用 handle_event 时也根据 revent 进行事件分发回调

    // 根据 revents 调用事件的回调函数
    std::function<void(Timestamp)> readCallback; // vector? callbacks
    std::function<void()> writeCallback;
    std::function<void()> closeCallback;
    std::function<void()> errorCallback;

    // 改变 channel 的 event 后，需要更新 channels 注册中心（在 poller），借助 loop 完成
    EventLoop* loop;







    // int index;          // used by poller
    std::weak_ptr<void> tiePtr;




private:
    void update(); // 当改变 channel 的 event 后，需要在 poller 里面更改（更新） channel。何时被调用：调用 enable_reading 等函数改变 event 后
    void handle_event_with_guard(Timestamp receiveTime); // 根据事件调用回调函数。何时被调用：被 handle_event() 调用

public:
    explicit Channel(EventLoop* loop, int fd);
    ~Channel();

    void handle_event(Timestamp receiveTime); // 核心函数，事件发生后进行回调。何时被调用：EventLoop 事件循环中被调用（先通过 poller 返回活动 channels）


    int get_fd() const { return fd; }

    uint32_t get_event() const { return event; }
    void enable_reading(); // 设置 fd 感兴趣的事件
    void disable_reading();
    void enable_writing();
    void disable_writing();
    void disable_all();

    void set_revent(uint32_t _revent) { revent = _revent; } // poller 监听到事件后设置此

    bool is_reading() const { return event & kReadEvent; } // 返回 fd 当前感兴趣的事件的状态，检查是否关注可读、可写、任何事件
    bool is_writing() const { return event & kWriteEvent; }
    bool is_none_event() const { return event == kNoneEvent; }

    void set_read_callback(std::function<void(Timestamp)> cb) { readCallback = std::move(cb); }
    void set_write_callback(std::function<void()> cb) { writeCallback = std::move(cb); }
    void set_close_callback(std::function<void()> cb) { closeCallback = std::move(cb); }
    void set_error_callback(std::function<void()> cb) { errorCallback = std::move(cb); }



    // int get_index() { return index; }
    // void set_index(int _index) { this->index = _index; }

    // 防止当 channel 被手动 remove 掉后，channel 还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    EventLoop* owner_loop() { return loop; }
    void remove();
};
