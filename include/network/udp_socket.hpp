#pragma once

#include <cstdint>

namespace network {

class UdpSocket {
public:
    UdpSocket();
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    int fd() const noexcept { return fd_; }

    void bind(uint16_t port);
    void set_non_blocking();
    void set_recv_buffer(int bytes);

private:
    int fd_{-1};
};

} // namespace network
