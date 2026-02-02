#pragma once

#include <pthread.h>
#include <vector>
#include <cstdint>

namespace lumina {

/// Pin current thread to a single CPU core to reduce context switches
/// and improve cache locality. Use with isolcpus for best effect.
inline bool pin_thread_to_core(uint32_t core_id) {
#ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  return pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0;
#else
  (void)core_id;
  return false;
#endif
}

/// Pin thread to a set of cores (e.g. one NUMA node).
inline bool pin_thread_to_cores(const std::vector<uint32_t>& core_ids) {
#ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  for (uint32_t c : core_ids)
    CPU_SET(c, &cpuset);
  return pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0;
#else
  (void)core_ids;
  return false;
#endif
}

} // namespace lumina
