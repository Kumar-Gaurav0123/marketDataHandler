#include "app/feed_handler.hpp"
#include "core/cpu_affinity.hpp"
#include "core/event_loop.hpp"
#include "network/multicast.hpp"
#include "network/udp_socket.hpp"

#include <sys/epoll.h>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
core::EventLoop* g_loop = nullptr;

void handle_signal(int) noexcept {
    if (g_loop) g_loop->stop();
}
} // namespace

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: feed_handler <port> <multicast_ip> [cpu_core]\n"
                  << "  e.g. feed_handler 12345 239.1.1.1 2\n";
        return 1;
    }

    const int port_arg = std::atoi(argv[1]);
    if (port_arg <= 0 || port_arg > 65535) {
        std::cerr << "Invalid port: " << argv[1] << '\n';
        return 1;
    }
    const uint16_t port     = static_cast<uint16_t>(port_arg);
    const char*    mcast_ip = argv[2];
    const int      cpu_core = argc > 3 ? std::atoi(argv[3]) : 0;

    if (cpu_core < 0) {
        std::cerr << "Invalid cpu_core: " << cpu_core << '\n';
        return 1;
    }

    std::signal(SIGINT,  handle_signal);
    std::signal(SIGTERM, handle_signal);

    try {
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

        std::cout << "[feed_handler] started"
                  << "  port="   << port
                  << "  mcast="  << mcast_ip
                  << "  cpu="    << cpu_core << '\n';

        loop.run();

        const auto& tob = handler.top_of_book();
        constexpr double scale = 1.0 / app::TopOfBook::PRICE_SCALE;

        std::cout << "[feed_handler] stopped\n"
                  << "  avg latency : " << handler.avg_latency_cycles() << " cycles\n"
                  << "  last seq_no : " << tob.seq_no.load() << '\n'
                  << "  bid         : " << tob.bid_price.load() * scale
                  << " x "             << tob.bid_size.load()  << '\n'
                  << "  ask         : " << tob.ask_price.load() * scale
                  << " x "             << tob.ask_size.load()  << '\n';

    } catch (const std::exception& ex) {
        std::cerr << "[feed_handler] fatal: " << ex.what() << '\n';
        return 1;
    }
}
