#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

namespace network {

class RecvBuffer {
public:
    static constexpr size_t MAX_PACKET_SIZE = 2048;
    static constexpr size_t BATCH_SIZE = 16;

    RecvBuffer();

    // Receives packets, returns number of packets read
    int recv(int socket_fd) noexcept;

    const uint8_t* data(size_t idx) const noexcept;
    size_t length(size_t idx) const noexcept;

private:
    alignas(64) std::array<std::array<uint8_t, MAX_PACKET_SIZE>, BATCH_SIZE> buffers_;
    std::array<size_t, BATCH_SIZE> lengths_;
};

} // namespace network
