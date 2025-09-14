#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Buffer.h"
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

#include "../base/Timestamp.h"
#include "../base/Log.h"

TcpConnection::TcpConnection(EventLoop* _loop, int _connFd)
    : loop(_loop)
    , connectFd(_connFd)
    , inputBuffer(new Buffer())
    , outputBuffer(new Buffer())
    , messageCallback(nullptr)
    , closeCallback(nullptr) {

    // 初始化 channel
    channel.reset(new Channel(_loop, _connFd)); // 也可以用 this->loop、this->connectFd, 因为初始化列表先执行。知道为什么初始化列表先执行吗？突然发现可能就是为了构造函数使用 this...
    channel->subscribe_on_read(std::bind(&TcpConnection::read_callback, this));
    channel->subscribe_on_write([this]() { this->write_callback(); });
    channel->subscribe_on_close([this]() { this->close_callback(); });
    channel->subscribe_on_error([this]() { this->error_callback(); });
    channel->enable_reading();
    loop->update_channel(channel.get());
}

TcpConnection::~TcpConnection() {
    // 虽然 connFd 是由 Acceptor 创建的，但是由 TcpConnection 持有，所以应该由其负责销毁
    int retClose = ::close(this->connectFd);
    if (retClose != 0) {
        LOG::LOG_ERROR("TcpConnection::~TcpConnection(). close fd failed, fd: ", this->connectFd);
    }
}

void TcpConnection::read_callback() {
    int savedErrno = 0;
    ssize_t n = inputBuffer->readFd(this->connectFd, &savedErrno);
    if (n > 0) {
        this->publish_message();
    }
    else if (n == 0) {
        this->close_callback();
    }
    else {
        std::cerr << "Read error: " << savedErrno << std::endl;
        this->close_callback();
    }
}

void TcpConnection::write_callback() {
    int savedErrno = 0;
    ssize_t n = outputBuffer->writeFd(this->connectFd, &savedErrno);
    if (n > 0) {
        if (outputBuffer->readable_bytes() == 0) {
            channel->disable_writing();
        }
    }
    else {
        std::cerr << "Write error: " << savedErrno << std::endl;
    }
}

void TcpConnection::close_callback() {
    channel->disable_all();
    loop->remove_channel(this->channel.get());

    this->publish_close();
}

void TcpConnection::error_callback() {
    channel->disable_all();
    loop->remove_channel(this->channel.get());

    this->publish_close();
}

void TcpConnection::publish_message() {
    if (messageCallback) {
        messageCallback(shared_from_this(), inputBuffer.get());
    }
    else LOG::LOG_ERROR("TcpConnection::publish_message(). no messageCallback setted.");
}

void TcpConnection::publish_close() {
    if (closeCallback) {
        closeCallback(shared_from_this());
    }
    else LOG::LOG_ERROR("TcpConnection::publish_close(). no closeCallback setted.");
}

void TcpConnection::subscribe_message(MessageCallback _cb) {
    this->messageCallback = std::move(_cb);
}

void TcpConnection::subscribe_close(CloseCallback _cb) {
    this->closeCallback = std::move(_cb);
}

void TcpConnection::send(const std::string& msg) {
    outputBuffer->append(msg);
    channel->enable_writing();
}

/* void TcpConnection::shutdown() {
    ::shutdown(this->connectFd, SHUT_WR);
} */