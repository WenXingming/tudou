/**
 * @file EventLoop.cpp
 * @brief 事件循环类，负责等待 I/O 事件，并进行相应的回调函数处理。不直接做 I/O
 * @author wenxingming
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include "EventLoop.h"
#include <iostream>
#include <sys/eventfd.h>
#include <sys/syscall.h> // 定义 SYS_gettid
#include <unistd.h>      // 定义 syscall, pid_t
#include <thread>
#include "../base/Timestamp.h"
#include "../base/Log.h"
#include "Poller.h"
#include "Channel.h"
#include <assert.h>

thread_local EventLoop* EventLoop::loopOfThisThread = nullptr;

int creat_event_fd() {
    int eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventFd < 0) {
        LOG::LOG_FATAL("create eventFd faild, error code: %d.", errno);
    }
    return eventFd;
}

EventLoop::EventLoop()
    : poller(Poller::new_default_poller(this))
    , pollTimeoutMs(5000)
    , threadId(static_cast<pid_t>(::syscall(SYS_gettid)))
    , isLooping(false)
    , isQuitedFlag(false)

    , callingPendingFunctors(false)
    , wakeupFd(creat_event_fd())
    , wakeupChannel(new Channel(this, wakeupFd))
    /* , currentActiveChannel(nullptr) */ {

    LOG::LOG_INFO("EventLoop created in thread %d", threadId);
    if (loopOfThisThread) {
        LOG::LOG_FATAL("Another EventLoop obj exists in this thread.");
    }
    else {
        loopOfThisThread = this;
    }

    // 每一个 EventLoop 都将监听 wakeupChannel 的 EPOLLIN 读事件
    wakeupChannel->set_read_callback([this](Timestamp timestamp) {
        handle_read();
        }); // std::bind(&EventLoop::handle_read(), this
    wakeupChannel->enable_reading(); // wakeupChannel 占用一个文件描述符：4
}

EventLoop::~EventLoop() {
    wakeupChannel->disable_all();
    wakeupChannel->remove();
    close(wakeupFd);

    loopOfThisThread = nullptr;
}

void EventLoop::loop() {
    isQuitedFlag = false;
    isLooping = true;
    LOG::LOG_INFO("EventLoop start looping...");
    while (!isQuitedFlag) {
        LOG::LOG_INFO("EventLoop is looping...");

        // 使用 poller 监听发生事件的 channels
        std::vector<Channel*> activeChannels = poller->poll(pollTimeoutMs);
        // 通知 channel 处理回调
        for (auto channel : activeChannels) {
            channel->handle_event(Timestamp::now()); // 也监听了 wakeupFd
        }

        // 执行当前 EventLoop 事件循环需要处理的回调操作
        // IO thread: mainLoop accept fd ---> get channel ---> subLoop
        // mainLoop 实现注册一个回调 cb，需要 subLoop 执行。wakeup subLoop 后，执行下面的方法，执行之前 mainLoop 注册的 cb 操作
        // do_pending_functors();
    }
    LOG::LOG_INFO("EventLoop stop looping.");
    isLooping = false;
}

void EventLoop::update_channel(Channel* channel) {
    poller->update_channel(channel);
}

void EventLoop::remove_channel(Channel* channel) {
    poller->remove_channel(channel);
}

bool EventLoop::has_channel(Channel* channel) {
    return poller->has_channel(channel);
}

EventLoop* EventLoop::get_loop_of_current_thread() {
    if (EventLoop::loopOfThisThread == nullptr) {
        LOG::LOG_ERROR("EventLoop::get_loop_of_current_thread(): loopOfThisThread == nullptr");
        assert(EventLoop::loopOfThisThread == nullptr);
        return EventLoop::loopOfThisThread;
    }
    else return EventLoop::loopOfThisThread;
}

bool EventLoop::is_in_loop_thread() const {
    return this->threadId == static_cast<pid_t>(::syscall(SYS_gettid));
}






void EventLoop::quit() {
    isQuitedFlag = true;

    if (is_in_loop_thread()) {
        wakeup();
    }
}

void EventLoop::run_in_loop(std::function<void()> _cb) {
    if (is_in_loop_thread()) {
        _cb();
    }
    else {
        queue_in_loop(_cb);
    }
}

void EventLoop::queue_in_loop(std::function<void()> _cb) {
    {
        std::unique_lock<std::mutex> uniqueLock(mutex);
        pendingFunctors.emplace_back(_cb);
    }
    if (!is_in_loop_thread() || callingPendingFunctors) {
        wakeup(); // 唤醒 loop 所在线程
    }
}

/// @brief 向 wakeupFd 写一个数据, wakeupChannel 就发生读事件，当前 loop 线程就会被唤醒
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG::LOG_ERROR("EventLoop::wakeup() write error.");
    }
}





void EventLoop::handle_read() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG::LOG_ERROR("EventLoop handle_read() error.");
    }
}

void EventLoop::do_pending_functors() {
    std::vector<std::function<void()>> functors;
    callingPendingFunctors = true;
    // 假设是耗时操作，避免长时间锁住 pendingFunctors
    {
        std::unique_lock<std::mutex> uniqueLock(mutex);
        functors.swap(pendingFunctors);
    }

    for (const auto& functor : functors) {
        functor();
    }
    callingPendingFunctors = false;
}
