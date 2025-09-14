// Buffer.h
#pragma once
#include <vector>
#include <string>

class Buffer {
public:
    Buffer(size_t initSize = 1024);

    size_t readable_bytes() const;
    size_t writable_bytes() const;

    const char* peek() const;       // 读指针
    void retrieve(size_t len);      // 读走 len 个字节
    void retrieveAll();             // 清空

    void append(const char* data, size_t len);  // 写入数据
    void append(const std::string& str);

    ssize_t readFd(int fd, int* savedErrno);    // 从 fd 读入
    ssize_t writeFd(int fd, int* savedErrno);   // 写入 fd

private:
    std::vector<char> buffer;
    size_t readIndex;
    size_t writeIndex;
};
