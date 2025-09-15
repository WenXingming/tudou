/**
 * @file Buffer.h
 * @brief
 * @details
 * @note My project address: https://github.com/WenXingming/Multi_IO
 */

#pragma once
#include <vector>
#include <string>

 /// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
 ///
 /// @code
 /// +-------------------+------------------+------------------+
 /// | prependable bytes |  readable bytes  |  writable bytes  |
 /// |                   |     (CONTENT)    |                  |
 /// +-------------------+------------------+------------------+
 /// |                   |                  |                  |
 /// 0      <=      readerIndex   <=   writerIndex    <=     size
 /// @endcode
 /// @details 设计的精巧之地在于，写入 buffer 后的数据自动转换为了可读这一角色。
 /// @details 例如：fd ==> buffer 后，数据转变为了可读区域，上层只需从可读区域取数据
 /// @details      上层 ==> buffer 后，数据转变为了可读区域，write_to_fd 也只需从可读区域取数据

class Buffer {
private:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    std::vector<char> buffer;
    size_t readIndex;
    size_t writeIndex;

private:
    void make_space(size_t len);

    size_t prependable_bytes() const;

    const char* readable_ptr() const;       // 读指针
    void retrieve(size_t len);              // 维护 index：读走 len 个字节
    void retrieve_all();                    // 维护 index：清空
    std::string retrieve_as_string(size_t len);              // 读走 len 个字节
    std::string retrieve_all_as_string();

public:
    explicit Buffer(size_t initSize = kInitialSize);

    size_t readable_bytes() const;
    size_t writable_bytes() const;

    std::string read_from_buffer();
    void write_to_buffer(const char* data, size_t len);
    void write_to_buffer(const std::string& str);

    ssize_t read_from_fd(int fd, int* savedErrno); // fd ==> buffer: 从 fd 读数据写入 buffer 的 writable 区域（写入 buffer）
    ssize_t write_to_fd(int fd, int* savedErrno);  // buffer ==> fd: 把 buffer 的 readable 区域写入 fd（从 buffer 读出）
};
