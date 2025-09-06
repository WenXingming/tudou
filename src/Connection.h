/**
 * @file Connection.h
 * @brief 连接进行抽象
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <sys/poll.h>
#include <sys/epoll.h>

/// @brief 每个 fdClient 连接的抽象
static const int BUFFER_LENGTH = 2048;
class Connection {
	int fd;
	char* readBuffer; // rbuffer + index、wbuffer + index
	int readIndex;
	char* writeBuffer;
	int writeIndex;

	bool isListenFd;

public:
	Connection(): fd(0), readIndex(0), writeIndex(0), isListenFd(false) {
		readBuffer = new char[BUFFER_LENGTH];
		writeBuffer = (char*)malloc(sizeof(char) * BUFFER_LENGTH);
	}
	~Connection() {
		if (isListenFd) // 关闭监听套接字（连接套接字在 recv 中有关闭逻辑）
			close(fd);
		if (readBuffer) delete[] readBuffer;
		if (writeBuffer) free(writeBuffer);
	}

	// 因为可能要复用 connection，所以仅有构造函数不够用
	void init(int _fd, bool _isListenFd = false) {
		fd = _fd;
		isListenFd = _isListenFd;
		memset(readBuffer, 0, BUFFER_LENGTH * sizeof(char));
		memset(writeBuffer, 0, BUFFER_LENGTH * sizeof(char));
		readIndex = 0;
		writeIndex = 0;
	}

	bool is_listen_fd() { return isListenFd; }

	int connect(int fdEpoll, Connection* connections); // 建立连接只需维护好 epoll、connections 即可
	int receive_data(int fdEpoll);
	int send_data(int fdEpoll);
};



inline int Connection::connect(int fdEpoll, Connection* connections) {
	sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);
	int fdClient = accept(fd, (sockaddr*)&clientAddr, &len); // 阻塞。其实不会阻塞，因为 if 已经说明了有连接请求
	if (fdClient == -1) {
		perror("accept failed!\n");
		return -1;
	}
	printf("fdClient: %d is accepted.\n", fdClient);

	// fdClient 注册到 epollfd
	epoll_event event;
	event.data.fd = fdClient;
	event.events = EPOLLIN;
	epoll_ctl(fdEpoll, EPOLL_CTL_ADD, event.data.fd, &event);

	// 初始化连接
	Connection& connection = connections[fdClient];
	connection.init(fdClient, false);

	return fdClient;
}


inline int Connection::receive_data(int fdEpoll) {
	int recvDataLength = recv(fd, readBuffer + readIndex, BUFFER_LENGTH - readIndex, 0);
	if (recvDataLength == -1) {
		perror("recv error!\n");
		return -1;
	}

	if (recvDataLength == 0) {
		printf("close clientfd.\n");
		close(fd);
		epoll_ctl(fdEpoll, EPOLL_CTL_DEL, fd, NULL);
		return -1;
	}

	readIndex += recvDataLength;

	// 处理数据：回声
	printf("recv buffer: %s\n", readBuffer); // \n 强制刷新行缓冲区，否则可能打印不出来。之前刷 b 站视频还真让我遇到了。debug 时记得加 \n！
	memcpy(writeBuffer, readBuffer, readIndex - 0);
	memset(readBuffer, 0, readIndex);
	writeIndex += readIndex;
	readIndex = 0;

	// 接受数据后设置监听事件为 EPOLLOUT
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLOUT;
	epoll_ctl(fdEpoll, EPOLL_CTL_MOD, event.data.fd, &event);

	return recvDataLength;
}

inline int Connection::send_data(int fdEpoll) {
	int sendDataLength = send(fd, writeBuffer, writeIndex - 0, 0);
	if (sendDataLength == -1) {
		perror("send error!\n");
		return -1;
	}

	writeIndex -= sendDataLength;

	// 发送数据后设置监听事件为 EPOLLIN
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	epoll_ctl(fdEpoll, EPOLL_CTL_MOD, event.data.fd, &event);

	return sendDataLength;
}
