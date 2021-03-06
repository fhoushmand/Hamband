#include "log.hpp"

namespace dory {

static bool is_zero(uint8_t *buf, size_t size) {
  // For very small buffers, we do not optimize
  if (size < 6 * sizeof(uint64_t)) {
    for (size_t i = 0; i < size; i++) {
      if (buf[i] != 0) {
        return false;
      }
    }

    return true;
  }

  // For larger buffers, we are guaranteed to be able to find an aligned segment
  // in the provided buffer. We use memcmp for this segment.

  // Get aligned start
  uint8_t *aligned_buf =
      reinterpret_cast<uint8_t *>((uintptr_t(buf) + sizeof(uint64_t) - 1) /
                                  sizeof(uint64_t) * sizeof(uint64_t));

  size_t unaligned_start_len = uintptr_t(aligned_buf) - uintptr_t(buf);
  size_t aligned_len =
      (size - unaligned_start_len) / sizeof(uint64_t) * sizeof(uint64_t);

  // Check unaligned start
  for (size_t i = 0; i < unaligned_start_len; i++) {
    if (buf[i] != 0) {
      return false;
    }
  }

  // Check unaligned end
  for (size_t i = unaligned_start_len + aligned_len; i < size; i++) {
    if (buf[i] != 0) {
      return false;
    }
  }

  // For this to work, we need an aligned buffer of at least 2 *
  // sizeof(uint64_t) bytes
  return *reinterpret_cast<uint64_t *>(aligned_buf) == 0 ||
         memcmp(aligned_buf, aligned_buf + sizeof(uint64_t),
                aligned_len - sizeof(uint64_t)) == 0;
}

Log::Log(void *underlying_buf, size_t buf_len)
    : buf{reinterpret_cast<uint8_t *>(underlying_buf)}, len{buf_len} {
  static_assert(LogConfig::is_powerof2(LogConfig::Alignment),
                "should use a power of 2 as template parameter");

  auto buf_addr = reinterpret_cast<uintptr_t>(underlying_buf);

  if (!is_zero(buf, buf_len)) {
    throw std::runtime_error("Provided buffer is not zeroed out");
  }

  auto offset = LogConfig::round_up_powerof2(buf_addr) - buf_addr;
  // std::cout << "Rounding up: " << LogConfig::round_up_powerof2(buf_addr) << " " <<
  // buf_addr << std::endl;
  if (offset > len) {
    throw std::runtime_error(
        "Alignment constraint leaves no space in the buffer");
  }

  // buf and len point at the beginning of the log. buf points to the
  // LogHeader.
  buf += offset;
  len -= offset;

  header = reinterpret_cast<LogHeader *>(buf);

  // header stores the following digest information about the state of the log:
  header->min_proposal = 0;
  header->first_undecided_offset = 0;
  header->free_bytes = len - LogConfig::round_up_powerof2(sizeof(LogHeader));

  offsets[MinProposal] =
      std::make_pair(reinterpret_cast<uint8_t *>(&(header->min_proposal)) -
                         reinterpret_cast<uint8_t *>(underlying_buf),
                     sizeof(header->min_proposal));
  offsets[FUO] = std::make_pair(
      reinterpret_cast<uint8_t *>(&(header->first_undecided_offset)) -
          reinterpret_cast<uint8_t *>(underlying_buf),
      sizeof(header->first_undecided_offset));
  offsets[Entries] =
      std::make_pair(len - header->free_bytes, dory::constants::MAX_ENTRY_SIZE);

  initial_fuo = len - header->free_bytes;
  initial_free_bytes = header->free_bytes;
  header->first_undecided_offset = initial_fuo;
}

Log::Entry Log::newEntry() {
  // std::cout << "Adding entry with absolute offset " << len -
  // header->free_bytes << std::endl;
  return Entry(buf + len - header->free_bytes, header->free_bytes);
}

void Log::finalizeEntry(Entry &entry) {
  auto bytes_used = entry.finalize();
  header->free_bytes -= LogConfig::round_up_powerof2(bytes_used);
}

std::vector<uint8_t> Log::dump() const {
  std::vector<uint8_t> v;

  for (size_t i = 0; i < len; i++) {
    v.push_back(buf[i]);
  }

  return v;
}




// Log for non-conflicting operations added by FARZIN

BandLog::BandLog(void *underlying_buf, size_t buf_len, size_t num_partitions)
    : buf{reinterpret_cast<uint8_t *>(underlying_buf)}, len{buf_len}, num_partitions{num_partitions} {
  static_assert(LogConfig::is_powerof2(LogConfig::Alignment),
                "should use a power of 2 as template parameter");

  auto buf_addr = reinterpret_cast<uintptr_t>(underlying_buf);

  if (!is_zero(buf, buf_len)) {
    throw std::runtime_error("Provided buffer is not zeroed out");
  }

  auto offset = LogConfig::round_up_powerof2(buf_addr) - buf_addr;
  // std::cout << "Rounding up: " << LogConfig::round_up_powerof2(buf_addr) << " " <<
  // buf_addr << std::endl;
  if (offset > len) {
    throw std::runtime_error(
        "Alignment constraint leaves no space in the buffer");
  }


  buf += offset;
  len -= offset;
  free_bytes = len;
}


BandLog::CallEntry BandLog::newCallEntry(uint64_t offset, bool override) {
  // std::cout << "Adding entry with absolute offset " << len -
  // header->free_bytes << std::endl;
  if(override)
    return CallEntry(offset + buf, free_bytes);
  else
    return CallEntry(offset + buf + len - free_bytes, free_bytes);
}

void BandLog::finalizeCallEntry(CallEntry &call) {
  auto bytes_used = call.finalize();
  free_bytes -= LogConfig::round_up_powerof2(bytes_used);
}

std::vector<uint8_t> BandLog::dump() const {
  std::vector<uint8_t> v;

  for (size_t i = 0; i < len; i++) {
    v.push_back(buf[i]);
  }

  return v;
}
}  // namespace dory