/**
 * @file Buffer.h
 * @brief 高效字节缓冲区，管理可读/可写/预留区域，并支持与 fd 的非阻塞读写。
 * @author
 * @project: https://github.com/WenXingming/tudou
 *
 */

#include "Buffer.h"
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <stdio.h>
#include <cassert>

#include "../base/Log.h"

Buffer::Buffer(size_t initSize)
    : buffer(kCheapPrepend + kInitialSize)
    , readIndex(kCheapPrepend)
    , writeIndex(kCheapPrepend) {}

size_t Buffer::readable_bytes() const {
    return writeIndex - readIndex;
}

size_t Buffer::writable_bytes() const {
    return buffer.size() - writeIndex;
}

size_t Buffer::prependable_bytes() const {
    return readIndex - 0;
}

const char* Buffer::readable_ptr() const {
    return buffer.data() + readIndex;
}

void Buffer::retrieve(size_t len) {
    if (len < readable_bytes()) { // 应用只读取了可读缓冲区的一部分
        readIndex += len;
    }
    else {
        retrieve_all();
    }
}

void Buffer::retrieve_all() {
    readIndex = kCheapPrepend;
    writeIndex = kCheapPrepend;
}

std::string Buffer::retrieve_as_string(size_t len) {
    // assert(len <= readable_bytes());
    if (len > readable_bytes())
        LOG::LOG_ERROR("Buffer::retrieve_as_string(). len > readable_bytes");

    len = std::min(len, readable_bytes());
    std::string str(readable_ptr(), len);
    retrieve(len);
    return std::move(str);
}

std::string Buffer::retrieve_all_as_string() {
    return retrieve_as_string(readable_bytes());
}

std::string Buffer::read_from_buffer() {
    auto readablePtr = readable_ptr();
    auto readableBytes = readable_bytes();
    std::string str(readablePtr, readableBytes);
    return std::move(str);
}

void Buffer::write_to_buffer(const char* data, size_t len) {
    if (len > writable_bytes()) {
        make_space(len); // 环形缓冲区：要么扩容要么调整
    }
    std::copy(data, data + len, buffer.begin() + writeIndex);
    writeIndex += len;
}

void Buffer::write_to_buffer(const std::string& str) {
    write_to_buffer(str.data(), str.size());
}

void Buffer::make_space(size_t len) {
    if (writable_bytes() + prependable_bytes() < len + kCheapPrepend) { // 环形缓冲区
        buffer.resize(writeIndex + len);
    }
    else { // 调整缓冲区
        int readableBytes = readable_bytes();
        std::copy(buffer.begin() + readIndex, buffer.begin() + writeIndex, buffer.begin() + kCheapPrepend);
        readIndex = kCheapPrepend;
        writeIndex = readIndex + readableBytes /* readable_bytes() */; // 不可直接调用
    }
}

/**
 * 从 fd 上读取数据，注意 events 是 LT 模式，数据没有读完也不会丢失。没有使用 while 读
 * 使用 iovec，buffer 不会太小也不会太大，完美利用内存
 */
ssize_t Buffer::read_from_fd(int fd, int* savedErrno) {
    char extraBuf[65536];
    size_t writableBytes = writable_bytes();

    struct iovec vec[2];
    vec[0].iov_base = buffer.data() + writeIndex;
    vec[0].iov_len = writableBytes;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    const int cnt = (writableBytes < sizeof(extraBuf)) ? 2 : 1;
    ssize_t n = ::readv(fd, vec, cnt);
    if (n < 0) {
        *savedErrno = errno;
    }
    else if (static_cast<size_t>(n) <= writableBytes) {
        writeIndex += n;
    }
    else {
        writeIndex = buffer.size();
        write_to_buffer(extraBuf, n - writableBytes);
    }
    return n;

    /* /// TODO: 设置 fd 为非阻塞，然后循环接受到 buffer
    char* start = this->buffer.data() + writeIndex;
    size_t length = this->writable_bytes();
    size_t n = ::recv(fd, start, length, 0);
    // if (n < 0) */
}

ssize_t Buffer::write_to_fd(int fd, int* savedErrno) {
    ssize_t n = ::write(fd, readable_ptr(), readable_bytes());
    if (n < 0) {
        *savedErrno = errno;
    }
    else {
        retrieve(n);
    }
    return n;
}
