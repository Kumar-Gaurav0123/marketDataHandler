#include <doctest/doctest.h>

#include "core/ring_buffer.hpp"

TEST_CASE("SpscRingBuffer - pop on empty returns false") {
    core::SpscRingBuffer<int, 4> buf;
    int val = 0;
    CHECK_FALSE(buf.pop(val));
}

TEST_CASE("SpscRingBuffer - push then pop returns value") {
    core::SpscRingBuffer<int, 4> buf;
    CHECK(buf.push(42));
    int val = 0;
    CHECK(buf.pop(val));
    CHECK(val == 42);
}

TEST_CASE("SpscRingBuffer - FIFO ordering") {
    core::SpscRingBuffer<int, 8> buf;
    for (int i = 0; i < 5; ++i) CHECK(buf.push(i));
    for (int i = 0; i < 5; ++i) {
        int val = -1;
        CHECK(buf.pop(val));
        CHECK(val == i);
    }
}

TEST_CASE("SpscRingBuffer - full buffer rejects push") {
    // Size=4, capacity=3 (one slot reserved for full/empty distinction)
    core::SpscRingBuffer<int, 4> buf;
    CHECK(buf.push(1));
    CHECK(buf.push(2));
    CHECK(buf.push(3));
    CHECK_FALSE(buf.push(4)); // full
}

TEST_CASE("SpscRingBuffer - push after pop on full buffer") {
    core::SpscRingBuffer<int, 4> buf;
    CHECK(buf.push(1));
    CHECK(buf.push(2));
    CHECK(buf.push(3));
    int val = 0;
    CHECK(buf.pop(val));
    CHECK(val == 1);
    CHECK(buf.push(99)); // should succeed now
}

TEST_CASE("SpscRingBuffer - works with non-trivial type") {
    core::SpscRingBuffer<std::pair<int,int>, 4> buf;
    CHECK(buf.push({1, 2}));
    std::pair<int,int> val{};
    CHECK(buf.pop(val));
    CHECK(val.first == 1);
    CHECK(val.second == 2);
}
