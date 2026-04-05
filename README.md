# marketDataHandler

![Build & Test](https://github.com/Kumar-Gaurav0123/marketDataHandler/actions/workflows/build.yml/badge.svg)

A low-latency market data feed handler written in C++17, targeting Linux x86-64. Designed for HFT infrastructure where microsecond-level performance matters.

Receives UDP multicast FIX market data, parses tag-value fields on the hot path, and maintains a live top-of-book (bid/ask price + size) with atomic visibility across threads.

## Features

| Component | Detail |
|---|---|
| **Event loop** | `epoll` edge-triggered, non-blocking — zero wasted syscalls |
| **Network** | UDP multicast with configurable kernel receive buffer |
| **Protocol** | Zero-allocation FIX tag-value parser (header-only, inlined) |
| **Order book** | Atomic top-of-book updated on every Market Data Snapshot / Incremental |
| **Ring buffer** | Lock-free SPSC with power-of-two sizing and cache-line isolation |
| **Latency** | Hot-path measured with serialized `rdtsc` (`lfence` + `rdtsc`) |
| **CPU affinity** | Thread pinned to a specific core via `pthread_setaffinity_np` |

## Requirements

- Linux x86-64 (kernel 2.6.17+)
- GCC ≥ 9 or Clang ≥ 10
- CMake 3.16+
- Ninja (optional, recommended)

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run:

```bash
# ./feed_handler <port> <multicast_ip> [cpu_core]
./build/feed_handler 12345 239.1.1.1 2
```

## Tests

```bash
cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build/debug --parallel
./build/debug/run_tests
```

Tests cover `FixParser` (valid messages, malformed input, overflow guard, edge cases) and `SpscRingBuffer` (FIFO ordering, full/empty, wraparound, non-trivial types).

## CI

Three jobs run on every push and pull request:

| Job | Compiler | Config |
|---|---|---|
| Release (GCC) | g++ | `-O3 -march=native -Werror` |
| ASan + UBSan | g++ | `-fsanitize=address,undefined -Werror` |
| Release (Clang) | clang++ | `-O3 -Werror` |

## Project Structure

```
marketDataHandler/
├── include/                         # Headers only
│   ├── app/
│   │   └── feed_handler.hpp         # FeedHandler + TopOfBook
│   ├── core/
│   │   ├── event_loop.hpp           # epoll event loop
│   │   ├── ring_buffer.hpp          # Lock-free SPSC ring buffer
│   │   ├── latency_tracker.hpp      # Atomic TSC latency tracker
│   │   ├── timestamp.hpp            # Serialized rdtsc
│   │   └── cpu_affinity.hpp         # pthread CPU affinity
│   ├── network/
│   │   ├── udp_socket.hpp           # Non-blocking UDP socket
│   │   ├── recv_buffer.hpp          # Batch packet receive (EPOLLET drain)
│   │   └── multicast.hpp            # IPv4 multicast group join
│   └── protocol/fix/
│       └── fix_parser.hpp           # Zero-allocation FIX parser
├── src/                             # Implementation files
│   ├── main.cpp                     # Entry point: arg parsing, signal handling
│   ├── app/feed_handler.cpp         # MsgType dispatch, top-of-book update
│   ├── core/
│   └── network/
└── tests/
    ├── test_fix_parser.cpp
    └── test_ring_buffer.cpp
```

## Design Notes

**Edge-triggered epoll (`EPOLLET`)** — The kernel only notifies once per readiness edge, not per datagram. `RecvBuffer::recv` loops with `MSG_DONTWAIT` until `EAGAIN` to fully drain the socket on each event, which is required for correctness under `EPOLLET`.

**Serialized `rdtsc`** — An `lfence` instruction before `rdtsc` prevents prior loads from being reordered past the counter read, giving accurate per-packet timestamps on the hot path without the overhead of `rdtscp`.

**SPSC ring buffer** — Power-of-two `Size` allows branchless index wrapping with `& mask_`. Head and tail are placed on separate cache lines (`alignas(64)`) to eliminate false sharing between producer and consumer cores.

**FIX parser** — Header-only template so the callback lambda is always inlined. Tag parsing uses an explicit overflow guard (`> MAX_TAG`) to prevent undefined behaviour on malformed input. Returns immediately on any malformed byte rather than attempting recovery.

**Top-of-book atomics** — Bid/ask fields are written with `memory_order_relaxed` and `seq_no` with `memory_order_release` last. A reader can validate consistency by checking `seq_no` has not changed across a read pair (seqlock-style, single writer).

**Memory ordering in `LatencyTracker`** — `record()` uses `relaxed` stores (no ordering needed on the hot path). `avg_cycles()` uses `acquire` loads to ensure it sees all committed increments before computing the average.

## License

MIT
