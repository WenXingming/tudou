#pragma once

#include <memory>

class Connection;
class EventLoop;
class RegistrationCenter;

// 虚基类 
class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void read_cb(Connection& conn);
    virtual void write_cb(Connection& conn);
    virtual void accept_cb(std::shared_ptr<RegistrationCenter> registeration, Connection& conn);
};


// 事件处理器，真正处理业务逻辑。我们只需要更改业务逻辑类即可
class EchoHandler : public EventHandler {
public:
    ~EchoHandler() override = default;
    void read_cb(Connection& conn) override;
    void write_cb(Connection& conn) override;
    void accept_cb(std::shared_ptr<RegistrationCenter> registeration, Connection& listenConn) override;
};