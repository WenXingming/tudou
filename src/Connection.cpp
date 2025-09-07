/**
 * @file Connection.cpp
 * @brief 事件源的抽象（fd/socket 等），用来唯一标识一个 I/O 资源。完全不依赖 epoll
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <sys/socket.h>
#include <errno.h>
#include <iostream>
#include "Connection.h"
#include "EventLoop.h"

static const bool debug = true;

Connection::Connection(int _fd, bool _isListenFd)
	: fd(_fd), isListenFd(_isListenFd), read_cb{ nullptr }, write_cb(nullptr), accept_cb(nullptr) {
	// 设置非阻塞
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

Connection::~Connection() {
	if (fd != -1) {
		close(fd);
		if (debug) printf("close fd: %d\n", fd);
	}
}

/// @brief EventLoop 回调函数
/// @details 先从 fd 读数据（到 buffer），再执行业务层的回调函数（把 buffer 数据拿到应用层处理业务）
/// @param loop 
void Connection::handle_read(EventLoop& loop) {
	// 和 fd 通信
	ssize_t ret = read_from_fd();
	if (ret == 0) {
		loop.remove_connection(fd); // loop 在这一层处理了，无需传入 read_cb
		return;
	}
	else if (ret < 0/* ret == -1 */) {
		std::cerr << "read error on fd " << fd << std::endl;
		loop.remove_connection(fd);
		return;
	}
	// ret > 0 表示读到了数据
	// 执行业务层回调函数
	if (read_cb) read_cb(*this);
}

/// @brief EventLoop 回调函数
/// @details 先执行业务层的回调函数（把业务层数据写入 buffer），再（从 buffer）向 fd 写数据
void Connection::handle_write() {
	// 执行业务层回调函数
	if (write_cb) write_cb(*this);
	// 和 fd 交互
	ssize_t ret = write_to_fd();
	if (ret < 0) {
		perror("write error.\n");
		return;
	}
}

// void Connection::handle_accept() {
void Connection::handle_accept(EventLoop& loop) {
	// if (accept_cb) accept_cb(*this);
	if (accept_cb) accept_cb(loop, *this);
}

/// @brief 
/// @return 读取的字节数。0 表示客户端断开连接，-1 表示错误
ssize_t Connection::read_from_fd() {
	char buf[2048];

	int totalSize = 0;
	while (true) { // 边缘触发（ET）模式，那更必须用循环，否则会丢事件；水平模式也用，否则剩下的数据要等下一轮 epoll 事件才会读，导致延迟增加，尤其是大消息/高吞吐场景。
		ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
		if (n > 0) {
			readBuffer.insert(readBuffer.end(), buf, buf + n);
			totalSize += n;
		}
		else if (n == 0) {
			return totalSize;
		}
		else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break; // 已读完
			}
			return -1; // 其它错误
		}
	}

	return totalSize;
}

/// @brief 
/// @return 写入的字节数。0 表示未写入，-1 表示错误
ssize_t Connection::write_to_fd() {
	ssize_t totalSize = 0;

	while (!writeBuffer.empty()) {
		ssize_t n = ::send(fd, writeBuffer.data() + totalSize, writeBuffer.size(), 0);
		if (n > 0) {
			totalSize += n;
		}
		else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break; // 内核缓冲区满，下次再写
			}
			return -1; // 其它错误
		}
	}
	writeBuffer.erase(writeBuffer.begin(), writeBuffer.begin() + totalSize);

	return totalSize;
}
