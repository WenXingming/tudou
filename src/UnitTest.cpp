/**
 * @file UnitTest.cpp
 * @brief 多路 IO 复用
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory>
#include <pthread.h>
#include <thread>
#include <poll.h>
#include <sys/poll.h>
#include <sys/epoll.h>

#include "Connection.h"


int main(int argc, char* argv[]) {
	// 创建监听套接字: socket()。
	int fdListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	assert(fdListen != -1);

	// 绑定监听 ip + port: bind()
	const char* ip = "127.0.0.1";
	const char* port = "2048";

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到所有网络接口（包括虚拟回环接口和物理网络接口）
	if (argc == 2)
		serverAddr.sin_port = htons(atoi(argv[1]));
	else
		serverAddr.sin_port = htons(atoi(port));

	int retBind = bind(fdListen, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (retBind == -1) {
		perror("bind failed!\n");
		return -1;
	}

	// 开始监听: listen()
	int retListen = listen(fdListen, 10);
	if (retListen == -1) {
		perror("listen failed!\n");
		getchar();
		return -1;
	}
	else {
		printf("fdListen: %d is listening...\n", fdListen);
	}

	// connections 初始化
	Connection connections[1024] = {};
	connections[fdListen].init(fdListen, true);

	/// DO: epoll 初始化
	int epfd = epoll_create(1); // 现在参数已经无用，> 0 即可（现使用链表无限扩展）。epfd 占用了 4 号 fd，所以 fdClient 从 5开始
	epoll_event event;
	event.data.fd = fdListen;
	event.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &event);

	// 一直处理 IO 多路复用事件
	epoll_event eventList[1024] = {};
	while (true) {
		int nready = epoll_wait(epfd, eventList, 1024, -1);

		for (int i = 0; i < nready; ++i) {
			int fd = eventList[i].data.fd;
			Connection& connection = connections[fd];

			if (connection.is_listen_fd()) {
				connection.connect(epfd, connections);
			}
			else {
				if (eventList[i].events & EPOLLIN) {
					connection.receive_data(epfd);
				}
				else {
					connection.send_data(epfd);
				}
			}
		}
	}

	getchar();
}