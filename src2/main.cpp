/**
 * @file main.cpp
 * @brief 测试
 * @author wenxingming
 * @date 2025-09-06
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#include "EventLoop.h"
#include "Channel.h"
#include "TcpServer.h"
#include "Buffer.h"
#include "TcpConnection.h"
#include "../base/Timestamp.h"
#include "../base/Log.h"
#include <iostream>
#include <unistd.h>
#include <cassert>
#include <thread>

 /**
  * @brief 直接使用网络库功能进行测试。还没有封装上层 TcpServer
  */
void test_netlib() {
    EventLoop loop;

    Channel stdinChannel(&loop, 0); // fd = 0 (标准输入)
    stdinChannel.enable_reading();
    stdinChannel.set_read_callback([&](Timestamp receivetime) {
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
 // void test_server() {
 //     EventLoop loop;
 //     TcpServer server(&loop, 8080);

 //     server.setMessageCallback(
 //         [](const std::shared_ptr<TcpConnection>& conn, Buffer* buf) {
 //             std::string msg(buf->peek(), buf->readable_bytes());
 //             buf->retrieveAll();
 //             std::cout << "Received: " << msg << std::endl;
 //             conn->send(msg); // echo 回去
 //         }
 //     );

 //     server.start();
 //     loop.loop();
 // }

int main() {
    // 日志系统
    LOG::disable_debug();

    std::thread t(test_netlib);
    t.join();
    return 0;
}

// #include "EventLoop.h"
// #include "TcpServer.h"
// #include "Buffer.h"
// #include "TcpConnection.h"
// #include <iostream>

// int main() {

// }

