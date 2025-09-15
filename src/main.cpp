/**
 * @file main.cpp
 * @brief 测试
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include <iostream>
#include <unistd.h>
#include <cassert>
#include <thread>

#include "../base/Timestamp.h"
#include "../base/Log.h"
#include "../base/InetAddress.h"
#include "Channel.h"
#include "Buffer.h"
#include "TcpConnection.h"

#include "EventLoop.h"
#include "TcpServer.h"

 /**
  * @brief 直接使用网络库功能进行测试。还没有封装上层 TcpServer
  */
void test_netlib() {
    EventLoop loop;

    Channel stdinChannel(&loop, 0); // fd = 0 (标准输入)
    stdinChannel.enable_reading();
    stdinChannel.subscribe_on_read([&](/* Timestamp receivetime */) {
        char buf[1024]{};
        ssize_t n = read(0, buf, sizeof(buf) - 1);
        if (n > 0) {
            std::cout << "stdin: " << buf << std::endl;
        }
        });
    loop.update_channel(&stdinChannel);

    loop.loop();
}


/**
 * @brief 测试上层 TcpServer
 */
void test_server() {
    EventLoop loop;

    InetAddress listenAddr = InetAddress(8080);
    TcpServer server(&loop, listenAddr);

    server.subscribe_message(
        [](const std::shared_ptr<TcpConnection>& conn) {
            std::string msg(conn->recv());
            std::cout << "Received: " << msg << std::endl;
            // 业务处理 ...
            // echo 回去
            conn->send(msg);
        }
    );

    server.start();
    loop.loop();
}

int main() {
    // 日志系统
    LOG::disable_debug();

    // std::thread t1(test_netlib);
    // t1.join();

    std::thread t2(test_server);
    t2.join();

    return 0;
}

// #include "EventLoop.h"
// #include "TcpServer.h"
// #include "Buffer.h"
// #include "TcpConnection.h"
// #include <iostream>

// int main() {

// }

