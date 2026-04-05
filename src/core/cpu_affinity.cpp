#include "core/cpu_affinity.hpp"

#ifdef __linux__

#include <pthread.h>
#include <sched.h>
#include <stdexcept>

namespace core {

void pin_thread_to_cpu(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);

    int rc = pthread_setaffinity_np(
        pthread_self(),
        sizeof(cpu_set_t),
        &cpuset
    );

    if (rc != 0) {
        throw std::runtime_error("Failed to set CPU affinity");
    }
}

} // namespace core

#endif // __linux__
