# marketDataHandler

A low-latency market data feed handler written in C++17, targeting Linux x86-64. Designed for high-frequency trading (HFT) infrastructure where microsecond-level performance matters.

## Features

- **epoll-based event loop** — edge-triggered, non-blocking I/O for minimal latency
- **UDP multicast support** — receive market data from multicast groups
- **FIX protocol parser** — lightweight tag-value parser with no heap allocations
- **Lock-free SPSC ring buffer** — single-producer single-consumer queue using atomics
- **TSC latency tracking** — nanosecond-resolution hot-path measurement via `rdtsc`
- **CPU affinity** — pin threads to specific cores to eliminate scheduling jitter

## Requirements

- Linux (kernel 2.6.17+ for epoll)
- x86-64 CPU
- GCC or Clang with C++17 support
- CMake 3.16+

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

For a debug build:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## Project Structure

```
marketDataHandler/
├── CMakeLists.txt
└── include/
    ├── app/
    │   ├── feed_handler.hpp      # Main feed handler (epoll event consumer)
    │   └── feed_handler.cpp
    ├── core/
    │   ├── event_loop.hpp        # epoll event loop
    │   ├── event_loop.cpp
    │   ├── ring_buffer.hpp       # Lock-free SPSC ring buffer
    │   ├── latency_tracker.hpp   # Atomic TSC-based latency tracker
    │   ├── timestamp.hpp         # Serialized rdtsc (lfence + rdtsc)
    │   ├── cpu_affinity.hpp      # Thread-to-core pinning
    │   └── cpu_affinity.cpp
    ├── network/
    │   ├── udp_socket.hpp        # Non-blocking UDP socket
    │   ├── udp_socket.cpp
    │   ├── recv_buffer.hpp       # Batch packet receive buffer
    │   ├── recv_buffer.cpp
    │   ├── multicast.hpp         # IPv4 multicast group join
    │   └── multicast.cpp
    └── protocol/fix/
        ├── fix_parser.hpp        # Zero-allocation FIX tag-value parser
        └── fix_parser.cpp
```

## Usage

```cpp
#include "core/event_loop.hpp"
#include "network/udp_socket.hpp"
#include "network/multicast.hpp"
#include "app/feed_handler.hpp"
#include "core/cpu_affinity.hpp"

int main() {
    core::pin_thread_to_cpu(2); // pin to core 2

    network::UdpSocket sock;
    sock.set_non_blocking();
    sock.set_recv_buffer(4 * 1024 * 1024); // 4 MB kernel buffer
    sock.bind(12345);

    network::join_multicast(sock.fd(), "239.1.1.1");

    app::FeedHandler handler(sock);

    core::EventLoop loop;
    loop.add_fd(sock.fd(), &handler, EPOLLIN | EPOLLET);
    loop.run();
}
```

## License

MIT
