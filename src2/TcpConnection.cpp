#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Buffer.h"
#include "../base/Timestamp.h"
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

TcpConnection::TcpConnection(EventLoop* _loop, int _sockfd)
    : loop(_loop)
    , connectFd(_sockfd),
    channel(new Channel(loop, this->connectFd)),
    inputBuffer(new Buffer()), outputBuffer(new Buffer()) {

    channel->set_read_callback([this](Timestamp receivetime) { handle_read(); });
    channel->set_write_callback([this]() { handle_write(); });
    channel->set_close_callback([this]() { handle_close(); });

    channel->enable_reading();

    loop->update_channel(channel.get());
}

TcpConnection::~TcpConnection() {
    ::close(this->connectFd);
}

void TcpConnection::send(const std::string& msg) {
    outputBuffer->append(msg);
    channel->enable_writing();
}

void TcpConnection::shutdown() {
    ::shutdown(this->connectFd, SHUT_WR);
}

void TcpConnection::handle_read() {
    int savedErrno = 0;
    ssize_t n = inputBuffer->readFd(this->connectFd, &savedErrno);
    if (n > 0) {
        if (messageCallback) {
            messageCallback(shared_from_this(), inputBuffer.get());
        }
    }
    else if (n == 0) {
        handle_close();
    }
    else {
        std::cerr << "Read error: " << savedErrno << std::endl;
        handle_close();
    }
}

void TcpConnection::handle_write() {
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

void TcpConnection::handle_close() {
    // channel->disable_all();
    if (closeCallback) {
        closeCallback(shared_from_this());
    }
}
