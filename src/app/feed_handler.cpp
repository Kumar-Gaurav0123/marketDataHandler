#include "app/feed_handler.hpp"

#include "core/timestamp.hpp"
#include "protocol/fix/fix_parser.hpp"

#include <sys/epoll.h>

namespace app {

FeedHandler::FeedHandler(network::UdpSocket& socket)
    : socket_(socket) {}

void FeedHandler::on_event(uint32_t events) noexcept {
    if (events & EPOLLIN) {
        int count = recv_buffer_.recv(socket_.fd());
        handle_packets(count);
    }
}

void FeedHandler::handle_packets(int count) noexcept {
    for (int i = 0; i < count; ++i) {
        const uint8_t* data = recv_buffer_.data(i);
        size_t len = recv_buffer_.length(i);

        // ---- HOT PATH START ----
        uint64_t start = core::rdtsc();

        protocol::fix::FixParser::parse(
            data, len,
            [](int tag, const uint8_t* value, size_t /*vlen*/) noexcept {
                // Tag 35 = MsgType
                if (tag == 35) {
                    (void)value;
                }
            }
        );

        uint64_t end = core::rdtsc();
        latency_.record(end - start);
        // ---- HOT PATH END ----
    }
}

} // namespace app
