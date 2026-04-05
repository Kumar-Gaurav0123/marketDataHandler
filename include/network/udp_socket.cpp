#ifdef __linux__

#include "network/udp_socket.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

namespace network {

UdpSocket::UdpSocket() {
    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0) {
        throw std::runtime_error("socket() failed");
    }

    int reuse = 1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#ifdef SO_REUSEPORT
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
#endif
}

UdpSocket::~UdpSocket() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

void UdpSocket::bind(uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw std::runtime_error("bind() failed");
    }
}

void UdpSocket::set_non_blocking() {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0 ||
        fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::runtime_error("fcntl(O_NONBLOCK) failed");
    }
}

void UdpSocket::set_recv_buffer(int bytes) {
    if (setsockopt(fd_, SOL_SOCKET, SO_RCVBUF,
                   &bytes, sizeof(bytes)) < 0) {
        throw std::runtime_error("SO_RCVBUF failed");
    }
}

} // namespace network

#endif // __linux__
