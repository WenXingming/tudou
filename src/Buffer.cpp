#include "Buffer.h"
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>

Buffer::Buffer(size_t initSize)
    : buffer(initSize), readIndex(0), writeIndex(0) {}

size_t Buffer::readable_bytes() const {
    return writeIndex - readIndex;
}

size_t Buffer::writable_bytes() const {
    return buffer.size() - writeIndex;
}

const char* Buffer::peek() const {
    return buffer.data() + readIndex;
}

void Buffer::retrieve(size_t len) {
    if (len < readable_bytes()) {
        readIndex += len;
    }
    else {
        retrieveAll();
    }
}

void Buffer::retrieveAll() {
    readIndex = 0;
    writeIndex = 0;
}

void Buffer::append(const char* data, size_t len) {
    if (writable_bytes() < len) {
        buffer.resize(writeIndex + len);
    }
    std::copy(data, data + len, buffer.begin() + writeIndex);
    writeIndex += len;
}

void Buffer::append(const std::string& str) {
    append(str.data(), str.size());
}

ssize_t Buffer::readFd(int fd, int* savedErrno) {
    char extraBuf[65536];
    struct iovec vec[2];
    size_t writable = writable_bytes();
    vec[0].iov_base = buffer.data() + writeIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    ssize_t n = ::readv(fd, vec, 2);
    if (n < 0) {
        *savedErrno = errno;
    }
    else if ((size_t)n <= writable) {
        writeIndex += n;
    }
    else {
        writeIndex = buffer.size();
        append(extraBuf, n - writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int* savedErrno) {
    ssize_t n = ::write(fd, peek(), readable_bytes());
    if (n < 0) {
        *savedErrno = errno;
    }
    else {
        retrieve(n);
    }
    return n;
}
