#pragma once

#include <cstdint>
#include <atomic>

namespace core {

class LatencyTracker {
public:
    void record(uint64_t cycles) noexcept {
        count_.fetch_add(1, std::memory_order_relaxed);
        total_.fetch_add(cycles, std::memory_order_relaxed);
    }

    uint64_t avg_cycles() const noexcept {
        uint64_t c = count_.load();
        return c ? total_.load() / c : 0;
    }

private:
    std::atomic<uint64_t> count_{0};
    std::atomic<uint64_t> total_{0};
};

} // namespace core
