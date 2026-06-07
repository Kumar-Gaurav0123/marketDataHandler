#pragma once

namespace core {

#ifdef __linux__
// Pin current thread to a specific CPU core
void pin_thread_to_cpu(int cpu_id);
#endif

} // namespace core
