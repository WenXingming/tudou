/**
 * @file EventLoop.h
 * @brief 事件循环，负责等待 I/O 事件，并把事件分发给合适的处理器。不直接做 I/O
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include "Connection.h"

class EventLoop {
	int epollFd;
	std::unordered_map<int, std::unique_ptr<Connection>> connections;

public:
	explicit EventLoop();
	~EventLoop();

	Connection& add_connection(int fd, bool isListenFd = false);
	void remove_connection(int fd);

	void loop();
};

