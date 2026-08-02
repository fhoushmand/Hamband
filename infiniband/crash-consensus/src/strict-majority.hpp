#pragma once

#include <cstddef>

namespace dory::crash_consensus {
inline constexpr std::size_t strictMajority(std::size_t total) {
  return total / 2 + 1;
}

inline constexpr int remoteStrictMajority(std::size_t remote_count) {
  return static_cast<int>(strictMajority(remote_count + 1) - 1);
}

inline constexpr int toleratedFailures(std::size_t total) {
  return static_cast<int>(total - strictMajority(total));
}
}  // namespace dory::crash_consensus
