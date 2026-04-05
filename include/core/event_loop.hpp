#pragma once

#ifdef __linux__

#include <sys/epoll.h>
#include <unistd.h>
#include <cstdint>
#include <array>
#include <unordered_map>

namespace core {

class EpollHandler {
public:
    virtual ~EpollHandler() = default;
    virtual void on_event(uint32_t events) noexcept = 0;
};

class EventLoop {
public:
    static constexpr int MAX_EVENTS = 64;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void add_fd(int fd, EpollHandler* handler, uint32_t events);
    void mod_fd(int fd, uint32_t events);
    void del_fd(int fd);

    void run() noexcept;
    void stop() noexcept;

private:
    int epoll_fd_{-1};
    bool running_{true};

    std::array<epoll_event, MAX_EVENTS> events_;
    std::unordered_map<int, EpollHandler*> handlers_;
};

} // namespace core

#endif // __linux__
