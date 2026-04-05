#pragma once

#include <cstdint>
#include <cstddef>

namespace protocol::fix {

// Lightweight, zero-allocation FIX tag-value parser.
// Format: <tag>=<value><SOH> repeated.
// Header-only: parse() is a hot-path template to allow inlining of the callback.
class FixParser {
public:
    static constexpr char SOH = '\x01';

    // Max valid FIX tag number per the spec (tags are 1-9999 in practice,
    // but allow up to 99999 to be safe — guard against int overflow).
    static constexpr int MAX_TAG = 99999;

    // Parse a FIX message buffer, calling on_field(tag, value_ptr, value_len)
    // for each well-formed field. Returns early on any malformed input.
    template <typename Callback>
    static void parse(const uint8_t* data,
                      size_t len,
                      Callback&& on_field) noexcept {
        size_t i = 0;

        while (i < len) {
            // --- parse tag (decimal digits, terminated by '=') ---
            int tag = 0;
            while (i < len && data[i] != '=') {
                if (data[i] < '0' || data[i] > '9') return; // non-digit: malformed
                tag = tag * 10 + (data[i] - '0');
                if (tag > MAX_TAG) return;                   // overflow guard
                ++i;
            }
            if (i >= len) return; // missing '='
            ++i;                  // skip '='

            // --- parse value (bytes until SOH) ---
            const uint8_t* value = &data[i];
            size_t value_len = 0;

            while (i < len && data[i] != SOH) {
                ++i;
                ++value_len;
            }
            if (i >= len) return; // missing SOH
            ++i;                  // skip SOH

            on_field(tag, value, value_len);
        }
    }
};

} // namespace protocol::fix
