#pragma once

#include <atomic>
#include <cstddef>
#include <array>
#include <type_traits>

namespace core {

template<typename T, size_t Size>
class alignas(64) SpscRingBuffer {
    static_assert((Size & (Size - 1)) == 0,
                  "Size must be power of two");

public:
    SpscRingBuffer() = default;

    bool push(const T& value) noexcept {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t next = (head + 1) & mask_;

        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // full
        }

        buffer_[head] = value;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& value) noexcept {
        const size_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return false; // empty
        }

        value = buffer_[tail];
        tail_.store((tail + 1) & mask_, std::memory_order_release);
        return true;
    }

private:
    static constexpr size_t mask_ = Size - 1;

    alignas(64) std::array<T, Size> buffer_;
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};
};

} // namespace core
