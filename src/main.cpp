#include "app/feed_handler.hpp"
#include "core/cpu_affinity.hpp"
#include "core/event_loop.hpp"
#include "network/multicast.hpp"
#include "network/udp_socket.hpp"

#include <sys/epoll.h>
#include <csignal>
#include <cstdlib>
#include <iostream>

namespace {
core::EventLoop* g_loop = nullptr;

void handle_signal(int) noexcept {
    if (g_loop) g_loop->stop();
}
} // namespace

int main(int argc, char* argv[]) {
    const uint16_t    port      = argc > 1 ? static_cast<uint16_t>(std::atoi(argv[1])) : 12345;
    const char*       mcast_ip  = argc > 2 ? argv[2] : "239.1.1.1";
    const int         cpu_core  = argc > 3 ? std::atoi(argv[3]) : 0;

    std::signal(SIGINT,  handle_signal);
    std::signal(SIGTERM, handle_signal);

    core::pin_thread_to_cpu(cpu_core);

    network::UdpSocket sock;
    sock.set_non_blocking();
    sock.set_recv_buffer(4 * 1024 * 1024); // 4 MB kernel buffer
    sock.bind(port);
    network::join_multicast(sock.fd(), mcast_ip);

    app::FeedHandler handler(sock);
    core::EventLoop  loop;
    g_loop = &loop;

    loop.add_fd(sock.fd(), &handler, EPOLLIN | EPOLLET);

    std::cout << "[feed_handler] listening"
              << "  port=" << port
              << "  mcast=" << mcast_ip
              << "  cpu="   << cpu_core << '\n';

    loop.run();

    std::cout << "[feed_handler] stopped."
              << "  avg latency=" << handler.avg_latency_cycles() << " cycles\n";
}
