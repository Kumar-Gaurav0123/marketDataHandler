#pragma once

#include <cstdint>

namespace core {

#if defined(__x86_64__) || defined(__i386__)
// Serialized TSC read
inline uint64_t rdtsc() noexcept {
    uint32_t lo, hi;
    asm volatile (
        "lfence\n\t"
        "rdtsc\n\t"
        : "=a"(lo), "=d"(hi)
        :
        : "memory"
    );
    return (static_cast<uint64_t>(hi) << 32) | lo;
}
#endif // __x86_64__ || __i386__

} // namespace core
