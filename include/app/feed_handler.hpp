#pragma once

#include "core/event_loop.hpp"
#include "core/latency_tracker.hpp"
#include "network/udp_socket.hpp"
#include "network/recv_buffer.hpp"

#include <cstdint>

namespace app {

class FeedHandler final : public core::EpollHandler {
public:
    explicit FeedHandler(network::UdpSocket& socket);

    void on_event(uint32_t events) noexcept override;

private:
    void handle_packets(int count) noexcept;

    network::UdpSocket& socket_;
    network::RecvBuffer recv_buffer_;
    core::LatencyTracker latency_;
};

} // namespace app
