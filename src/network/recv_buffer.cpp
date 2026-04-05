#include "network/recv_buffer.hpp"

#include <sys/socket.h>
#include <errno.h>
#include <cstring>

namespace network {

RecvBuffer::RecvBuffer() {
    lengths_.fill(0);
}

int RecvBuffer::recv(int socket_fd) noexcept {
    int count = 0;

    while (count < static_cast<int>(BATCH_SIZE)) {
        ssize_t n = ::recvfrom(
            socket_fd,
            buffers_[count].data(),
            MAX_PACKET_SIZE,
            MSG_DONTWAIT,
            nullptr,
            nullptr
        );

        if (n > 0) {
            lengths_[count] = static_cast<size_t>(n);
            ++count;
        } else {
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                break; // drained socket (EPOLLET requirement)
            }
            break;
        }
    }

    return count;
}

const uint8_t* RecvBuffer::data(size_t idx) const noexcept {
    return buffers_[idx].data();
}

size_t RecvBuffer::length(size_t idx) const noexcept {
    return lengths_[idx];
}

} // namespace network
