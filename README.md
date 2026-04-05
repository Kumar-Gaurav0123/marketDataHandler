# marketDataHandler

![Build & Test](https://github.com/Kumar-Gaurav0123/marketDataHandler/actions/workflows/build.yml/badge.svg)

A low-latency market data feed handler written in C++17, targeting Linux x86-64. Designed for high-frequency trading (HFT) infrastructure where microsecond-level performance matters.

## Features

- **epoll-based event loop** — edge-triggered, non-blocking I/O for minimal latency
- **UDP multicast support** — receive market data from multicast groups
- **FIX protocol parser** — lightweight zero-allocation tag-value parser (header-only)
- **Lock-free SPSC ring buffer** — single-producer single-consumer queue using atomics
- **TSC latency tracking** — nanosecond-resolution hot-path measurement via `rdtsc`
- **CPU affinity** — pin threads to specific cores to eliminate scheduling jitter

## Requirements

- Linux (kernel 2.6.17+)
- x86-64 CPU
- GCC or Clang with C++17 support
- CMake 3.16+
- Ninja (optional, recommended)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

Run the feed handler:

```bash
./feed_handler [port] [multicast_ip] [cpu_core]
# e.g.
./feed_handler 12345 239.1.1.1 2
```

## Tests

Unit tests are built automatically with the project:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --parallel
./run_tests
```

Coverage includes `FixParser` (valid messages, malformed input, edge cases) and `SpscRingBuffer` (FIFO ordering, full/empty, wraparound).

## Project Structure

```
marketDataHandler/
├── CMakeLists.txt
├── include/                        # Headers only
│   ├── app/
│   │   └── feed_handler.hpp        # Feed handler interface
│   ├── core/
│   │   ├── event_loop.hpp          # epoll event loop
│   │   ├── ring_buffer.hpp         # Lock-free SPSC ring buffer
│   │   ├── latency_tracker.hpp     # Atomic TSC-based latency tracker
│   │   ├── timestamp.hpp           # Serialized rdtsc (lfence + rdtsc)
│   │   └── cpu_affinity.hpp        # Thread-to-core pinning
│   ├── network/
│   │   ├── udp_socket.hpp          # Non-blocking UDP socket
│   │   ├── recv_buffer.hpp         # Batch packet receive buffer
│   │   └── multicast.hpp           # IPv4 multicast group join
│   └── protocol/fix/
│       └── fix_parser.hpp          # Zero-allocation FIX tag-value parser
├── src/                            # Implementation files
│   ├── main.cpp
│   ├── app/
│   ├── core/
│   └── network/
└── tests/
    ├── test_fix_parser.cpp
    └── test_ring_buffer.cpp
```

## Design Notes

- **Edge-triggered epoll (`EPOLLET`)** requires fully draining the socket on each event — `RecvBuffer::recv` handles this with a batch loop.
- **`lfence` before `rdtsc`** prevents out-of-order execution from contaminating latency measurements on the hot path.
- **SPSC ring buffer** uses power-of-two sizing for branchless index wrapping (`& mask`), with `alignas(64)` on head/tail to avoid false sharing.
- **FIX parser** is entirely branch-based with no heap allocation, making it suitable for the packet processing hot path.

## License

MIT
