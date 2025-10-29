/**
 * @file EventLoop.h
 * @brief 事件循环（Reactor）核心类，驱动 I/O 事件的收集与回调执行。
 * @author wenxingming
 * @project: https://github.com/WenXingming/tudou
 * @details
 *
 * 说明：
 * - 封装一个 Poller（如 EpollPoller），负责注册/取消/修改文件描述符的事件并等待就绪事件。（持有 poller 的唯一所有权（std::unique_ptr），在 EventLoop 析构时自动释放）
 * - 提供 loop() 方法进入事件循环，持续调用 poller 获取 Active Channels
 * - 对获取到的 Active Channels，调用其 publish_events() 方法，触发相应的事件回调
 * - 对外暴露 update_channel()/remove_channel() 方法，用于 Channel 向 EventLoop 注册或取消自身。
 *
 * 线程模型与约定：
 * - EventLoop 继承自 NonCopyable，禁止拷贝与赋值以避免多个所有者。
 * - EventLoop 通常与线程一一绑定（一个线程一个 EventLoop），非线程安全方法须在所属线程调用。
 *
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
    const int pollTimeoutMs; // poller 默认的超时时间

public:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop();

    void update_channel(Channel* channel);
    void remove_channel(Channel* channel);
};
