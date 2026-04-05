#include "app/feed_handler.hpp"

#include "core/timestamp.hpp"
#include "protocol/fix/fix_parser.hpp"

#include <sys/epoll.h>
#include <cstring>

// FIX tag constants used for market data processing
namespace fix_tags {
    constexpr int MsgType      = 35;
    constexpr int MsgSeqNum    = 34;
    constexpr int MDEntryType  = 269; // '0'=Bid, '1'=Offer, '2'=Trade
    constexpr int MDEntryPx    = 270;
    constexpr int MDEntrySize  = 271;
} // namespace fix_tags

// FIX MsgType values
namespace fix_msg_type {
    constexpr char MarketDataSnapshot[]   = "W";
    constexpr char MarketDataIncremental[] = "X";
    constexpr char Trade[]                = "AE";
} // namespace fix_msg_type

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

        // Peek at MsgType (tag 35) to dispatch to the right handler.
        // We do a quick scan rather than a full parse to keep the fast path lean.
        char msg_type[8] = {};
        bool found = false;

        protocol::fix::FixParser::parse(data, len,
            [&](int tag, const uint8_t* value, size_t vlen) noexcept {
                if (!found && tag == fix_tags::MsgType && vlen < sizeof(msg_type)) {
                    std::memcpy(msg_type, value, vlen);
                    found = true;
                }
            });

        if (found) {
            if (std::memcmp(msg_type, fix_msg_type::MarketDataSnapshot,   1) == 0 ||
                std::memcmp(msg_type, fix_msg_type::MarketDataIncremental, 1) == 0) {
                process_market_data(data, len);
            } else if (std::memcmp(msg_type, fix_msg_type::Trade, 2) == 0) {
                process_trade(data, len);
            }
        }

        uint64_t end = core::rdtsc();
        latency_.record(end - start);
        // ---- HOT PATH END ----
    }
}

// Parse a market data message and update top-of-book atomically.
void FeedHandler::process_market_data(const uint8_t* data, size_t len) noexcept {
    int64_t  bid_px   = 0, ask_px   = 0;
    int64_t  bid_sz   = 0, ask_sz   = 0;
    uint64_t seq_no   = 0;
    char     side     = 0;   // '0'=Bid '1'=Ask from MDEntryType

    protocol::fix::FixParser::parse(data, len,
        [&](int tag, const uint8_t* value, size_t vlen) noexcept {
            if (vlen == 0) return;

            switch (tag) {
                case fix_tags::MsgSeqNum: {
                    uint64_t n = 0;
                    for (size_t j = 0; j < vlen; ++j) n = n * 10 + (value[j] - '0');
                    seq_no = n;
                    break;
                }
                case fix_tags::MDEntryType:
                    side = static_cast<char>(value[0]);
                    break;

                case fix_tags::MDEntryPx: {
                    // Parse fixed-point: "123.45" -> 1234500 (4 d.p. scale)
                    int64_t integer = 0, frac = 0, frac_digits = 0;
                    bool in_frac = false;
                    for (size_t j = 0; j < vlen; ++j) {
                        if (value[j] == '.') { in_frac = true; continue; }
                        if (in_frac) { frac = frac * 10 + (value[j] - '0'); ++frac_digits; }
                        else         { integer = integer * 10 + (value[j] - '0'); }
                    }
                    // Normalise to PRICE_SCALE (4 d.p.)
                    int64_t scaled = integer * TopOfBook::PRICE_SCALE;
                    for (int k = static_cast<int>(frac_digits); k < 4; ++k) frac *= 10;
                    for (int k = static_cast<int>(frac_digits); k > 4; --k) frac /= 10;
                    scaled += frac;

                    if (side == '0') bid_px = scaled;
                    else if (side == '1') ask_px = scaled;
                    break;
                }
                case fix_tags::MDEntrySize: {
                    int64_t sz = 0;
                    for (size_t j = 0; j < vlen; ++j) sz = sz * 10 + (value[j] - '0');
                    if (side == '0') bid_sz = sz;
                    else if (side == '1') ask_sz = sz;
                    break;
                }
                default: break;
            }
        });

    // Publish atomically — seq_no is written last so a reader can detect
    // in-progress updates by checking seq_no before and after the read.
    if (bid_px) { tob_.bid_price.store(bid_px, std::memory_order_relaxed);
                  tob_.bid_size .store(bid_sz, std::memory_order_relaxed); }
    if (ask_px) { tob_.ask_price.store(ask_px, std::memory_order_relaxed);
                  tob_.ask_size .store(ask_sz, std::memory_order_relaxed); }
    if (seq_no) tob_.seq_no.store(seq_no, std::memory_order_release);
}

// Parse a trade execution message (AE) — currently records the seq_no only.
// Extend here to feed a trade tape / last-sale price.
void FeedHandler::process_trade(const uint8_t* data, size_t len) noexcept {
    protocol::fix::FixParser::parse(data, len,
        [&](int tag, const uint8_t* value, size_t vlen) noexcept {
            if (tag == fix_tags::MsgSeqNum && vlen > 0) {
                uint64_t n = 0;
                for (size_t j = 0; j < vlen; ++j) n = n * 10 + (value[j] - '0');
                tob_.seq_no.store(n, std::memory_order_release);
            }
        });
}

} // namespace app
