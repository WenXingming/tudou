/**
 * @file EventLoop.h
 * @brief 事件循环类，负责时间循环
 * @author wenxingming
 * @date 2025-09-09
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <sys/syscall.h> // 定义 SYS_gettid
#include <unistd.h>      // 定义 syscall, pid_t
#include "../base/NonCopyable.h"
#include "Poller.h"


class Channel;
class Timestamp;
class EventLoop : NonCopyable {
private:
    static thread_local EventLoop* loopOfThisThread; // 防止一个线程创建多个 EventLoop

private:
    std::unique_ptr<Poller> poller; // 拥有 poller。智能指针，自动析构
    const int pollTimeoutMs = 10000; // poller 默认的超时时间

    const pid_t threadId; // 记录当前 loop 所在的线程 id。is_in_loop_thread() 使用
    std::atomic<bool> isLooping;
    std::atomic<bool> isQuitedFlag;

public:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop();

    void update_channel(Channel* channel);
    void remove_channel(Channel* channel);
    bool has_channel(Channel* channel);



    static EventLoop* get_loop_of_current_thread(); // 返回当前线程的 eventloop（可能为空）
    bool is_in_loop_thread() const;







private:

    // std::unique_ptr<TimerQueue> timerQueue;


    Timestamp pollReturnTimestamp;

    int wakeupFd; // eventfd: wait\notify。当 mainLoop 获取新用户的 channel，通过 RR 选择一个 subReactor，通过该成员唤醒 subReactor 线程
    std::unique_ptr<Channel> wakeupChannel;

    // std::vector<Channel*> activeChannels;
    // Channel* currentActiveChannel;

    std::atomic<bool> callingPendingFunctors; // 标识当前 loop 是否有需要执行的回调
    std::vector<std::function<void()>> pendingFunctors; // 存储 loop 需要执行的所有回调操作
    std::mutex mutex;

private:
    void handle_read();
    void do_pending_functors();




public:
    void quit();

    Timestamp poll_return_time() const { return pollReturnTimestamp; }

    /// @brief run cb in current loop
    void run_in_loop(std::function<void()> _cb);
    /// @brief set cb in queue
    void queue_in_loop(std::function<void()> _cb);

    void wakeup();


};
