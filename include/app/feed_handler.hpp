#pragma once

#ifdef __linux__

#include "core/event_loop.hpp"
#include "core/latency_tracker.hpp"
#include "network/udp_socket.hpp"
#include "network/recv_buffer.hpp"

#include <cstdint>
#include <atomic>

namespace app {

// Aggregated top-of-book for a single instrument.
// Written by the feed handler thread only; reads may come from other threads.
struct TopOfBook {
    std::atomic<int64_t>  bid_price{0};  // scaled: price * PRICE_SCALE
    std::atomic<int64_t>  ask_price{0};
    std::atomic<int64_t>  bid_size{0};
    std::atomic<int64_t>  ask_size{0};
    std::atomic<uint64_t> seq_no{0};

    static constexpr int64_t PRICE_SCALE = 10000; // 4 d.p. fixed-point
};

class FeedHandler final : public core::EpollHandler {
public:
    explicit FeedHandler(network::UdpSocket& socket);

    void on_event(uint32_t events) noexcept override;

    uint64_t avg_latency_cycles() const noexcept { return latency_.avg_cycles(); }
    const TopOfBook& top_of_book() const noexcept { return tob_; }

private:
    void handle_packets(int count) noexcept;
    void process_market_data(const uint8_t* data, size_t len) noexcept;
    void process_trade(const uint8_t* data, size_t len) noexcept;

    network::UdpSocket& socket_;
    network::RecvBuffer recv_buffer_;
    core::LatencyTracker latency_;
    TopOfBook tob_;
};

} // namespace app

#endif // __linux__
