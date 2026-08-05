#pragma once

#include <cstdint>
#include <string>

namespace state_digest {

inline uint64_t mix(uint64_t value) {
  value += 0x9e3779b97f4a7c15ULL;
  value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
  value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
  return value ^ (value >> 31);
}

inline uint64_t string(std::string const& value) {
  uint64_t hash = 1469598103934665603ULL;
  for (unsigned char byte : value) {
    hash ^= byte;
    hash *= 1099511628211ULL;
  }
  return hash;
}

inline void addUnordered(uint64_t& digest, uint64_t value) {
  digest += mix(value);
}

inline void addOrdered(uint64_t& digest, uint64_t value) {
  digest = mix(digest ^ mix(value));
}

}  // namespace state_digest
