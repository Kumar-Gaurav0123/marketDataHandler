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
        // acquire pairs with the relaxed fetch_add stores above — ensures we
        // see fully committed values and not a torn read across count/total.
        uint64_t c = count_.load(std::memory_order_acquire);
        uint64_t t = total_.load(std::memory_order_acquire);
        return c ? t / c : 0;
    }

private:
    alignas(64) std::atomic<uint64_t> count_{0};
    alignas(64) std::atomic<uint64_t> total_{0};
};

} // namespace core
