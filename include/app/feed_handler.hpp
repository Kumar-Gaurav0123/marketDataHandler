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
    uint64_t avg_latency_cycles() const noexcept { return latency_.avg_cycles(); }

private:
    void handle_packets(int count) noexcept;

    network::UdpSocket& socket_;
    network::RecvBuffer recv_buffer_;
    core::LatencyTracker latency_;
};

} // namespace app
