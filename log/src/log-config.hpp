#pragma once

#include <dory/shared/units.hpp>

namespace dory {
using dory::units::operator""_GiB;
struct LogConfig {
  static constexpr int Alignment = 64;
  static constexpr size_t LOG_SIZE = 1_GiB;

  static constexpr bool is_powerof2(size_t v) {
    return v && ((v & (v - 1)) == 0);
  }

  static constexpr size_t round_up_powerof2(size_t v) {
    return (v + Alignment - 1) & (-static_cast<ssize_t>(Alignment));
  }
};
}  // namespace dory