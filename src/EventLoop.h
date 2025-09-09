/**
 * @file EventLoop.h
 * @brief 事件循环类，负责等待 I/O 事件，并进行相应的回调函数处理。不直接做 I/O
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once

#include <memory>

class RegistrationCenter;
class EventLoop {
    std::shared_ptr<RegistrationCenter> registrationCenter;

public:
    explicit EventLoop(std::shared_ptr<RegistrationCenter> ptr);
    ~EventLoop() {}
    EventLoop(const EventLoop& other) = delete;
    EventLoop operator=(const EventLoop& other) = delete;

    void loop();
};
