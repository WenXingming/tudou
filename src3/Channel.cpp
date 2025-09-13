// #include "Channel.h"
// #include <sys/epoll.h>

// Channel::Channel(int _fd)
//     : fd(_fd),
//     event(0),
//     revent(0) {

// }

// void Channel::handle_event() {
//     if ((revent & EPOLLHUP) && !(revent & EPOLLIN)) {
//         if (closeCallback) closeCallback();
//         return;
//     }
//     if (revent & (EPOLLERR)) {
//         if (closeCallback) closeCallback();
//         return;
//     }
//     if (revent & (EPOLLIN | EPOLLPRI)) {
//         if (readCallback) readCallback();
//     }
//     if (revent & EPOLLOUT) {
//         if (writeCallback) writeCallback();
//     }
// }