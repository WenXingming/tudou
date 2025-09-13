#pragma once
#include <functional>
#include <cstdint>
#include <sys/epoll.h>

class Channel {
private:
    int fd; // 不拥有 fd，不负责 close(fd)
    uint32_t event;     // 感兴趣的事件
    uint32_t revent;    // poller 实际监听到的事件

public:
    explicit Channel(int fd);
    ~Channel() = default;

    void handle_event(); // 由 EventLoop 调用
    
};

// public:
//     using EventCallback = std::function<void()>;

//     explicit Channel(int fd);
//     ~Channel() = default;

//     int get_fd() const { return fd; }
//     uint32_t get_event() const { return event; }
//     void set_revent(uint32_t _revent) { revent = _revent; }

//     void enable_reading() { event |= EPOLLIN; }
//     void enable_writing() { event |= EPOLLOUT; }
//     void disable_writing() { event &= ~EPOLLOUT; }

//     void set_read_callback(EventCallback cb) { readCallback = std::move(cb); }
//     void set_write_callback(EventCallback cb) { writeCallback = std::move(cb); }
//     void set_close_callback(EventCallback cb) { closeCallback = std::move(cb); }

//     void handle_event(); // 由 EventLoop 调用

// private:
//     int fd;
//     uint32_t event;     // 感兴趣的事件
//     uint32_t revent;    // poller 实际监听到的事件

//     EventCallback readCallback;
//     EventCallback writeCallback;
//     EventCallback closeCallback;