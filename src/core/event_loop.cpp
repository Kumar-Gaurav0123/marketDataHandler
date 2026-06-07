#include "core/event_loop.hpp"

#ifdef __linux__

#include <stdexcept>
#include <cstring>
#include <errno.h>

namespace core {

EventLoop::EventLoop() {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        throw std::runtime_error("epoll_create1 failed");
    }
}

EventLoop::~EventLoop() {
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
    }
}

void EventLoop::add_fd(int fd, EpollHandler* handler, uint32_t events) {
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = handler;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl ADD failed");
    }
    handlers_[fd] = handler;
}

void EventLoop::mod_fd(int fd, uint32_t events) {
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = handlers_.at(fd);

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl MOD failed");
    }
}

void EventLoop::del_fd(int fd) {
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        throw std::runtime_error("epoll_ctl DEL failed");
    }
    handlers_.erase(fd);
}

void EventLoop::run() noexcept {
    while (running_) {
        int n = epoll_wait(epoll_fd_,
                           events_.data(),
                           MAX_EVENTS,
                           -1);

        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }

        for (int i = 0; i < n; ++i) {
            auto* handler =
                static_cast<EpollHandler*>(events_[i].data.ptr);
            handler->on_event(events_[i].events);
        }
    }
}

void EventLoop::stop() noexcept {
    running_ = false;
}

} // namespace core

#endif // __linux__
