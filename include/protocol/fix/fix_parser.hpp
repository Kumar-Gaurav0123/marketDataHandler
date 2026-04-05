#pragma once

#include <cstdint>
#include <cstddef>

namespace protocol::fix {

// Very small FIX tag-value parser
// Format: tag=value<SOH>
class FixParser {
public:
    static constexpr char SOH = '\x01';

    // Parse FIX message buffer
    // Calls on_field(tag, value, len) for each field
    template <typename Callback>
    static void parse(const uint8_t* data,
                      size_t len,
                      Callback&& on_field) noexcept {
        size_t i = 0;

        while (i < len) {
            // Parse tag (digits only, stop at '=')
            int tag = 0;
            while (i < len && data[i] != '=') {
                if (data[i] < '0' || data[i] > '9') return; // malformed
                tag = tag * 10 + (data[i] - '0');
                ++i;
            }
            if (i >= len) return; // no '=' found
            ++i; // skip '='

            const uint8_t* value = &data[i];
            size_t value_len = 0;

            while (i < len && data[i] != SOH) {
                ++i;
                ++value_len;
            }
            if (i >= len) return; // no SOH found
            ++i; // skip SOH

            on_field(tag, value, value_len);
        }
    }
};

} // namespace protocol::fix
