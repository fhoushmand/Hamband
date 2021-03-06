#include <iostream>

#include "log-iterators.hpp"
#include "log-config.hpp"

using namespace std;


namespace dory {
PartitionedIterator::PartitionedIterator(uint8_t* base_ptr, ptrdiff_t part_length, size_t num_partition) {
        this->base_ptr = base_ptr;
        this->part_length = part_length;
        this->num_partitions = num_partition;
        entry_ptrs.resize(num_partitions);
        for(size_t i = 0; i < num_partition; i++){
          entry_ptrs[i] = base_ptr + (i * part_length) + 1024;
          // std::cout << "offset " << i << " " << offset(i) << std::endl;
        }   
      }

void PartitionedIterator::reset(size_t partition) {
  entry_ptrs[partition] = base_ptr + (partition * part_length);
}

PartitionedIterator& PartitionedIterator::next(size_t partition) {
  auto length = *reinterpret_cast<uint64_t*>(entry_ptrs[partition]);
  auto canary = entry_ptrs[partition] + length + sizeof(uint64_t);
  while (*canary == 0);
  entry_ptrs[partition] += dory::LogConfig::round_up_powerof2(canary + 1 - entry_ptrs[partition]);

  return *this;
}
}  // namespace dory

namespace dory {
SnapshotIterator::SnapshotIterator(uint8_t* entry_ptr, size_t length)
    : entry_ptr{entry_ptr}, end_ptr{entry_ptr + length} {}

SnapshotIterator& SnapshotIterator::next() {
  auto length = *reinterpret_cast<uint64_t*>(entry_ptr);
  auto canary = entry_ptr + length + sizeof(uint64_t);

  if (*canary != 0xff) {
    throw std::runtime_error("Missing canary value on the snapshot iterator");
  }

  entry_ptr += LogConfig::round_up_powerof2(canary + 1 - entry_ptr);

  return *this;
}
}  // namespace dory

namespace dory {
BlockingIterator::BlockingIterator(uint8_t* entry_ptr)
    : saved_entry_ptr{entry_ptr}, entry_ptr{entry_ptr}, increment{0} {}

void BlockingIterator::reset() { 
  entry_ptr = saved_entry_ptr;
  increment = 0; 
}

BlockingIterator& BlockingIterator::next() {
  entry_ptr += increment;

  volatile auto length_ptr = reinterpret_cast<uint64_t*>(entry_ptr);
  while (*length_ptr == 0) {
    ;
  }

  volatile auto canary_ptr = entry_ptr + *length_ptr + sizeof(uint64_t);
  while (*canary_ptr == 0) {
    ;
  }

  increment = LogConfig::round_up_powerof2(canary_ptr + 1 - entry_ptr);
  return *this;
}

bool BlockingIterator::sampleNext() {
  auto tmp_entry_ptr = entry_ptr + increment;

  // check for the length of the entry which is the first uint64_t 
  volatile auto length_ptr = reinterpret_cast<uint64_t*>(tmp_entry_ptr);
  if (*length_ptr == 0) {
    return false;
  }

  // checks if the canary bit has been set
  volatile auto canary_ptr = tmp_entry_ptr + *length_ptr + sizeof(uint64_t);
  if (*canary_ptr == 0) {
    return false;
  }

  entry_ptr = tmp_entry_ptr;
  // finds the nearest power of 2 of the entry length. (entry length is calculated as: canary_ptr + 1 - entry_ptr)
  increment = LogConfig::round_up_powerof2(canary_ptr + 1 - entry_ptr);
  return true;
}
}  // namespace dory

namespace dory {
LiveIterator::LiveIterator(uint8_t* base_ptr, uint8_t* entry_ptr)
    : base_ptr{base_ptr}, entry_ptr{entry_ptr}, increment{0} {}

void LiveIterator::reset() { increment = 0; }

bool LiveIterator::hasNext(ptrdiff_t limit) {
  entry_ptr += increment;
  increment = 0;
  // printf("limit: %ld \t entry_prt:%p \t base_ptr: %p\n",limit, static_cast<void*>(entry_ptr), static_cast<void*>(base_ptr));
  return entry_ptr < base_ptr + limit;
}

LiveIterator& LiveIterator::next(bool check) {
  entry_ptr += increment;

  volatile auto length_ptr = reinterpret_cast<uint64_t*>(entry_ptr);
  if (check) {
    while (*length_ptr == 0) {
      ;
    }
  }

  volatile auto canary_ptr = entry_ptr + *length_ptr + sizeof(uint64_t);
  if (check) {
    while (*canary_ptr == 0) {
      ;
    }
  }

  increment = LogConfig::round_up_powerof2(canary_ptr + 1 - entry_ptr);
  return *this;
}
}  // namespace dory

namespace dory {
RemoteIterator::RemoteIterator(size_t entry_header_size)
    : predictor{entry_header_size} {}

RemoteIterator::RemoteIterator(int remote_id, size_t remote_offset,
                               size_t entry_header_size)
    : remote_id{remote_id},
      remote_offset{remote_offset},
      prev_remote_offset{remote_offset},
      predictor{entry_header_size} {}

std::pair<ptrdiff_t, size_t> RemoteIterator::lookAt(uint64_t rem_offset) {
  if (rem_offset != remote_offset) {
    prev_remote_offset = remote_offset;
  }

  remote_offset = rem_offset;
  remote_size = predictor.predict();
  return std::make_pair(remote_offset, remote_size);
}

bool RemoteIterator::isPopulated(uint8_t* data, size_t length) {
  if (length < sizeof(uint64_t)) {
    throw std::runtime_error(
        "First make sure that you `canMove`, before calling "
        "`isEntryComplete`.");
  }

  auto length_ptr = reinterpret_cast<uint64_t*>(data);
  return *length_ptr != 0;
}

bool RemoteIterator::canMove(uint8_t* data, size_t length) {
  auto length_ptr = reinterpret_cast<uint64_t*>(data);

  // std::cout << "Length entry " << *length_ptr << std::endl;

  if (*length_ptr + 1 + sizeof(uint64_t) <= length) {
    // std::cout << "True: Adjusting the predictor to " << *length_ptr + 1 +
    // sizeof(uint64_t) << std::endl;
    predictor.adjust(*length_ptr + 1 + sizeof(uint64_t));
    return true;
  }

  // std::cout << "False: Adjusting the predictor to " << *length_ptr + 1 +
  // sizeof(uint64_t) << std::endl;
  predictor.adjust(*length_ptr + 1 + sizeof(uint64_t));
  return false;
}

bool RemoteIterator::isEntryComplete(uint8_t* data, size_t length) {
  auto length_ptr = reinterpret_cast<uint64_t*>(data);

  if (*length_ptr + 1 + sizeof(uint64_t) > length) {
    throw std::runtime_error(
        "First make sure that you `canMove`, before calling "
        "`isEntryComplete`.");
  }

  auto canary_ptr = data + *length_ptr + sizeof(uint64_t);
  return *canary_ptr != 0;
}
}  // namespace dory