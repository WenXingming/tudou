/**
 * @file main.cpp
 * @brief 测试
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <cassert>
#include "EventLoop.h"
#include "EventHander.h"

int main() {
	int listenFd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenFd != -1);

	sockaddr_in addr{};
	int port = 2048;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	int retBind = bind(listenFd, (sockaddr*)&addr, sizeof(addr));
	assert(retBind != -1);

	int retListen = listen(listenFd, SOMAXCONN);
	assert(retListen != -1);


	EventLoop loop;
	Connection& listenConn = loop.add_connection(listenFd, true);

	// listenConn.set_accept_cb([&loop](Connection& listenConn) {
	// listenConn.set_accept_cb([](EventLoop& loop, Connection& listenConn) {
	// 	sockaddr_in clientAddr{};
	// 	socklen_t len = sizeof(clientAddr);
	// 	int clientFd = accept(listenConn.get_fd(), (sockaddr*)&clientAddr, &len);
	// 	std::cout << "accept new client: " << clientFd << std::endl;

	// 	Connection& clientConn = loop.add_connection(clientFd);
	// 	clientConn.set_read_cb([](Connection& conn) {
	// 		auto& buf = conn.get_read_buffer();
	// 		std::string msg(buf.begin(), buf.end());
	// 		std::cout << "recv: " << msg << std::endl;

	// 		// Echo 回写
	// 		conn.get_write_buffer().insert(
	// 			conn.get_write_buffer().end(),
	// 			buf.begin(),
	// 			buf.end()
	// 		);
	// 		buf.clear();
	// 		});
	// 	clientConn.set_write_cb([](Connection& conn) {
	// 		std::cout << "send done on fd " << conn.get_fd() << std::endl;
	// 		});
	// 	});
	EchoHandler handler;
	listenConn.set_accept_cb([&handler](EventLoop& loop, Connection& listenConn) {
		handler.accept_cb(loop, listenConn);
		});

	loop.loop();
}
