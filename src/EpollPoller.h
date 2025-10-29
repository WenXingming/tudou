/**
 * @file EpollPoller.h
 * @brief 基于 epoll 的 Poller 实现 — 多路 I/O 事件分发器（Reactor 的 I/O 多路复用层）
 * @author wenxingming
 * @project: https://github.com/WenXingming/tudou
 * @details
 *
 * 说明：
 * - 封装 epoll 系统调用（epoll_create1 / epoll_ctl / epoll_wait），作为 Poller 的 epoll 实现。
 * - 维护 fd -> Channel* 的映射（注册中心），将内核返回的就绪事件翻译为 Channel 列表返回给 EventLoop。
 * - 负责对 Channel 的 ADD/MOD/DEL 操作的映射与执行（调用 epoll_ctl）。
 *
 * 设计要点：
 * - 非线程安全：所有操作应在所属 EventLoop 所在线程执行（除非上层做了线程同步）。
 * - 内部维护一个可增长的 eventList（std::vector<epoll_event>），当返回就绪数量达到容量时自动扩容以避免丢事件。
 * - 不拥有 Channel（仅保存裸指针映射），Channel 的生存期由上层（Acceptor / TcpConnection 等）管理。
 *
 * 注意：
 * - eventListSize 成员用于初始化 eventList 的初始容量；声明顺序保证先初始化此常量。
 * - 为了与 Poller 接口兼容，类对外提供 poll/update_channel/remove_channel 三个操作。
 */

#include "Poller.h"
#include <vector>
#include <sys/epoll.h>
#include <unordered_map>

class EpollPoller : public Poller {
private:
    // fd -> Channel* 映射，作为注册中心（不拥有 Channel）
    std::unordered_map<int, Channel*> channels;

    int epollFd;
    const size_t eventListSize = 16; // 初始事件数组大小；声明顺序保证先初始化此常量
    std::vector<epoll_event> eventList; // 存放 epoll_wait 返回的就绪事件列表

private:
    std::vector<Channel*> get_activate_channels(int numEvents) const;
    void event_list_auto_resize(int numReady);

public:
    explicit EpollPoller(EventLoop* _loop); // 构造时创建 epoll fd
    ~EpollPoller() override;

    std::vector<Channel*> poll(int timeoutMs) override;

    void update_channel(Channel* channel) override;
    void remove_channel(Channel* channel) override;
};
