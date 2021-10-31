#pragma once

#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>

#include <array>
#include <utility>

#include "log-config.hpp"
#include "log-constants.hpp"
#include "log-helpers.hpp"
#include "log-iterators.hpp"

namespace dory {
class ParsedSlot {
 public:
  ParsedSlot(uint8_t* ptr) : ptr{ptr} {}

  inline uint64_t acceptedProposal() {
    return *reinterpret_cast<uint64_t*>(ptr + offsets[1]);
  }

  inline void setAcceptedProposal(uint64_t proposal) {
    *reinterpret_cast<uint64_t*>(ptr + offsets[1]) = proposal;
  }

  inline uint64_t firstUndecidedOffset() {
    return *reinterpret_cast<uint64_t*>(ptr + offsets[2]);
  }

  inline std::pair<uint8_t*, size_t> payload() {
    auto length =
        *reinterpret_cast<uint64_t*>(ptr + offsets[0]) - 2 * sizeof(uint64_t);
    auto buf = ptr + offsets[3];
    return std::make_pair(buf, length);
  }

  inline bool isPopulated() { return *reinterpret_cast<uint64_t*>(ptr) > 0; }

  inline size_t totalLength() {
    return *reinterpret_cast<uint64_t*>(ptr) + sizeof(uint64_t) + 1;
  }

  static inline size_t copy(uint8_t* dst, uint8_t* src) {
    auto src_len = *reinterpret_cast<uint64_t*>(src);
    auto size = src_len + 1 + sizeof(uint64_t);
    if (src != dst) {
      memcpy(dst, src, size);
    }

    return size;
  }

 private:
  static constexpr const int offsets[] = {
      0,   // For the length of the entry,
      8,   // For the acceptedProposal,
      16,  // For the firstUndecidedOffset,
      24   // For the buffer
  };

  uint8_t* ptr;
};


class ParsedCall {
 public:
  ParsedCall() {}
  ParsedCall(uint8_t* ptr) : ptr{ptr} {}

  inline int* dependencies() {
    return reinterpret_cast<int*>(ptr + offsets[2]);
  }

  inline bool hasDeps() {
    return *reinterpret_cast<int*>(ptr + offsets[1]);
  }

  inline void setDependencies(int* deps) {

    int* first = deps;
    int* last = deps + 4;
    int* d_first = reinterpret_cast<int*>(ptr + offsets[1]);

    while (first != last) {
        *d_first++ = *first++;
    }
  }

  inline std::pair<uint8_t*, size_t> payload() {
    if(hasDeps()){
      auto length = *reinterpret_cast<uint64_t*>(ptr + offsets[0]) - 4 * sizeof(int);
      auto buf = ptr + offsets[2] + (4 * sizeof(int));  
      return std::make_pair(buf, length);
    }
    else{
      auto length = *reinterpret_cast<uint64_t*>(ptr + offsets[0]) - sizeof(int);
      auto buf = ptr + offsets[2];
      return std::make_pair(buf, length);
    }
  }

  inline bool isPopulated() { return *reinterpret_cast<uint64_t*>(ptr) > 0; }

  // inline size_t totalLength() {
  //   return *reinterpret_cast<uint64_t*>(ptr) + sizeof(uint64_t) + 1;
  // }

  // static inline size_t copy(uint8_t* dst, uint8_t* src) {
  //   auto src_len = *reinterpret_cast<uint64_t*>(src);
  //   auto size = src_len + 1 + sizeof(uint64_t);
  //   if (src != dst) {
  //     memcpy(dst, src, size);
  //   }

  //   return size;
  // }

 private:
  static constexpr const int offsets[] = {
      0,   // For the length of the entry,
      8,   // Flag for deps,
      12  // Deps/buffer,
  };

  uint8_t* ptr;
};


// Log for non-conflicting calls(wellcoordination -> Hamsaz) added by FARZIN 
class HamsazLog {
 public:
  class CallEntry {
   public:
    CallEntry() {}

    CallEntry(uint8_t* start, size_t remaining_space)
        : base{start},
          start{start + sizeof(uint64_t)},
          space{remaining_space},
          len{0} {}

    inline void store_buf(const void* buf, size_t length) {
      if (len + length > space) {
        throw std::runtime_error("Log ran out of space. Entry cannot fit.");
      }

      memcpy(start, buf, length);
      start += length;
      len += length;
    }

    inline void indicate_no_deps() {
      // indicates dependency exists
      *reinterpret_cast<int*>(start) = 0;
      start += sizeof(int);
      len += sizeof(int);
    }

    inline void store_deps(const int* deps, size_t length = 4) {
      if (len + length > space) {
        throw std::runtime_error("Log ran out of space. Entry cannot fit.");
      }
      // indicates dependency exists
      *reinterpret_cast<int*>(start) = 1;
      start += sizeof(int);
      len += sizeof(int);

      // actual deps counters
      for(size_t i = 0; i < length; i++)
      {
        *reinterpret_cast<int*>(start) = deps[i];
        start += sizeof(int);
        len += sizeof(int);
      }
      // *reinterpret_cast<int*>(start) = deps[1];
      // start += sizeof(int);
      // len += sizeof(int);
      // *reinterpret_cast<int*>(start) = deps[2];
      // start += sizeof(int);
      // len += sizeof(int);
    }

    inline size_t finalize() {
      auto length = reinterpret_cast<uint64_t*>(start - len - sizeof(uint64_t));
      *length = len;
      *start = 0xff;

      // The +1 is for the canary value, the uint64_t is because we encode the
      // the length.
      return *length + 1 + sizeof(uint64_t);
    }

    inline uint8_t* basePtr() const { return base; }

    inline size_t length() const { return len + 1 + sizeof(uint64_t); }

   private:
    uint8_t* base;
    uint8_t* start;
    uint64_t space;
    uint64_t len;
  };

  HamsazLog(void* underlying_buf, size_t buf_len, size_t num_partitions);

  // Copy constructor
  HamsazLog(HamsazLog const& other) = delete;

  // Copy assignment operator
  HamsazLog& operator=(HamsazLog const& other) = delete;

  // Move constructor
  HamsazLog(HamsazLog&& other) = delete;

  // Move assignment operator
  HamsazLog& operator=(HamsazLog&& other) = delete;

  // Destructor
  ~HamsazLog() = default;

  CallEntry newCallEntry(uint64_t offset, bool override);
  void finalizeCallEntry(CallEntry& call);

  inline uint8_t* bufPtr() const {
    return reinterpret_cast<uint8_t*>(buf);
  }

  // inline bool spaceLeftCritical() {
  //   return spaceLeft() < constants::CRITICAL_LOG_FREE_SPACE;
  // }

  


  PartitionedIterator partitionedIterator() {
    return PartitionedIterator(buf, LogConfig::round_up_powerof2(len/num_partitions) , num_partitions);
  }

  std::vector<uint8_t> dump() const;
  
 private:
  uint64_t free_bytes;
  uint8_t* buf;
  size_t len;
  size_t num_partitions;
};

class Log {
 public:
  class Entry {
   public:
    Entry() {}

    Entry(uint8_t* start, size_t remaining_space)
        : base{start},
          start{start + sizeof(uint64_t)},
          space{remaining_space},
          len{0} {}

    inline void fast_store(uint64_t const x, uint64_t const y, uint8_t* buf,
                           size_t buf_len) {
      auto temp = start;

      *reinterpret_cast<uint64_t*>(start) = x;
      start += sizeof(x);

      *reinterpret_cast<uint64_t*>(start) = y;
      start += sizeof(y);

      memcpy(start, buf, buf_len);
      start += buf_len;

      len += start - temp;
    }

    inline void store_uint64(uint64_t const& x) {
      if (len + sizeof(x) > space) {
        throw std::runtime_error("Log ran out of space. Entry cannot fit.");
      }

      *reinterpret_cast<uint64_t*>(start) = x;
      start += sizeof(x);
      len += sizeof(x);
    }

    inline void store_buf(const void* buf, size_t length) {
      if (len + length > space) {
        throw std::runtime_error("Log ran out of space. Entry cannot fit.");
      }

      memcpy(start, buf, length);
      start += length;
      len += length;
    }

    inline size_t finalize() {
      auto length = reinterpret_cast<uint64_t*>(start - len - sizeof(uint64_t));
      *length = len;
      *start = 0xff;

      // The +1 is for the canary value, the uint64_t is because we encode the
      // the length.
      return *length + 1 + sizeof(uint64_t);
    }

    inline uint8_t* basePtr() const { return base; }

    inline size_t length() const { return len + 1 + sizeof(uint64_t); }

   private:
    uint8_t* base;
    uint8_t* start;
    uint64_t space;
    uint64_t len;
  };


  // class CallEntry {
  //  public:
  //   CallEntry() {}

  //   CallEntry(uint8_t* start, size_t remaining_space)
  //       : base{start},
  //         start{start + sizeof(uint64_t)},
  //         space{remaining_space},
  //         len{0} {}

  //   // inline void fast_store(uint64_t const x, uint64_t const y, uint8_t* buf,
  //   //                        size_t buf_len) {
  //   //   auto temp = start;

  //   //   *reinterpret_cast<uint64_t*>(start) = x;
  //   //   start += sizeof(x);

  //   //   *reinterpret_cast<uint64_t*>(start) = y;
  //   //   start += sizeof(y);

  //   //   memcpy(start, buf, buf_len);
  //   //   start += buf_len;

  //   //   len += start - temp;
  //   // }

  //   // inline void store_uint64(uint64_t const& x) {
  //   //   if (len + sizeof(x) > space) {
  //   //     throw std::runtime_error("Log ran out of space. Entry cannot fit.");
  //   //   }

  //   //   *reinterpret_cast<uint64_t*>(start) = x;
  //   //   start += sizeof(x);
  //   //   len += sizeof(x);
  //   // }

  //   inline void store_buf(const void* buf, size_t length) {
  //     if (len + length > space) {
  //       throw std::runtime_error("Log ran out of space. Entry cannot fit.");
  //     }

  //     memcpy(start, buf, length);
  //     start += length;
  //     len += length;
  //   }

  //   inline void indicate_no_deps() {
  //     // indicates dependency exists
  //     *reinterpret_cast<int*>(start) = 0;
  //     start += sizeof(int);
  //     len += sizeof(int);
  //   }

  //   inline void store_deps(const int* deps, size_t length = 3*4) {
  //     if (len + length > space) {
  //       throw std::runtime_error("Log ran out of space. Entry cannot fit.");
  //     }
  //     // indicates dependency exists
  //     *reinterpret_cast<int*>(start) = 1;
  //     start += sizeof(int);
  //     len += sizeof(int);

  //     // actual deps counters
  //     *reinterpret_cast<int*>(start) = deps[0];
  //     start += sizeof(int);
  //     len += sizeof(int);
  //     *reinterpret_cast<int*>(start) = deps[1];
  //     start += sizeof(int);
  //     len += sizeof(int);
  //     *reinterpret_cast<int*>(start) = deps[2];
  //     start += sizeof(int);
  //     len += sizeof(int);
      
  //     // memcpy(start, buf, length);
  //     // start += length;
  //     // len += length;
  //   }

  //   inline size_t finalize() {
  //     auto length = reinterpret_cast<uint64_t*>(start - len - sizeof(uint64_t));
  //     *length = len;
  //     *start = 0xff;

  //     // The +1 is for the canary value, the uint64_t is because we encode the
  //     // the length.
  //     return *length + 1 + sizeof(uint64_t);
  //   }

  //   inline uint8_t* basePtr() const { return base; }

  //   inline size_t length() const { return len + 1 + sizeof(uint64_t); }

  //  private:
  //   uint8_t* base;
  //   uint8_t* start;
  //   uint64_t space;
  //   uint64_t len;
  // };

  struct LogHeader {
    uint64_t min_proposal;
    uint64_t first_undecided_offset;
    uint64_t free_bytes;
  };

  enum Offsets {
    MinProposal = 0,
    FUO = 1,
    Entries = 2,
  };

  Log(void* underlying_buf, size_t buf_len);

  // Copy constructor
  Log(Log const& other) = delete;

  // Copy assignment operator
  Log& operator=(Log const& other) = delete;

  // Move constructor
  Log(Log&& other) = delete;

  // Move assignment operator
  Log& operator=(Log&& other) = delete;

  // Destructor
  ~Log() = default;

  inline uint8_t* headerPtr() const {
    return reinterpret_cast<uint8_t*>(header);
  }

  inline std::pair<ptrdiff_t, size_t>& offset(Offsets off) {
    return offsets[off];
  }

  Entry newEntry();
  void finalizeEntry(Entry& entry);

  // CallEntry newCallEntry(uint64_t offset);
  // void finalizeCallEntry(CallEntry& call);

  inline uint64_t headerProposalAddress() volatile {
    uint64_t volatile* prop = &(header->min_proposal);
    return *prop;
  }

  inline void updateHeaderProposal(uint64_t proposal) volatile {
    uint64_t volatile* prop = &(header->min_proposal);
    *prop = proposal;
  }

  inline uint64_t headerFirstUndecidedOffset() volatile {
    uint64_t volatile* off = &(header->first_undecided_offset);
    return *off;
  }

  inline void rebuildLog() volatile {
    auto fuo = headerFirstUndecidedOffset();

    header->free_bytes = len - fuo;
    auto offset = reinterpret_cast<uint8_t*>(header) + fuo;

    while (true) {
      ParsedSlot pslot(offset);
      if (pslot.isPopulated()) {
        auto bytes_used = LogConfig::round_up_powerof2(pslot.totalLength());
        offset += bytes_used;
        header->free_bytes -= bytes_used;
      } else {
        break;
      }
    }
  }

  inline size_t spaceLeft() { return header->free_bytes; }

  inline bool spaceLeftCritical() {
    return spaceLeft() < constants::CRITICAL_LOG_FREE_SPACE;
  }

  inline void resetFUO() { header->first_undecided_offset = initial_fuo; }

  inline void bzero() {
    memset(headerPtr() + initial_fuo, 0, initial_free_bytes);
  }

  inline uint8_t* firstUndecidedOffsetEntry() volatile {
    return reinterpret_cast<uint8_t*>(header) + headerFirstUndecidedOffset();
  }

  inline void updateHeaderFirstUndecidedOffset(uint64_t offset) volatile {
    uint64_t volatile* off = &(header->first_undecided_offset);
    *off = offset;
  }

  // PartitionedIterator partitionedIterator(size_t num_partitions) {
  //   return PartitionedIterator(buf + LogConfig::round_up_powerof2(sizeof(LogHeader)), len/num_partitions, num_partitions);
  // }

  SnapshotIterator snapshotIterator() {
    // std::cout << "Offset: " << len - header->free_bytes - sizeof(LogHeader)
    //           << std::endl;
    return SnapshotIterator(
        buf + LogConfig::round_up_powerof2(sizeof(LogHeader)),
        len - header->free_bytes -
            LogConfig::round_up_powerof2(sizeof(LogHeader)));
  }

  BlockingIterator blockingIterator() {
    return BlockingIterator(buf +
                            LogConfig::round_up_powerof2(sizeof(LogHeader)));
  }

  LiveIterator liveIterator() {
    return LiveIterator(buf,
                        buf + LogConfig::round_up_powerof2(sizeof(LogHeader)));
  }

  RemoteIterator remoteIterator(int remote_id, uintptr_t offset = 0) {
    if (offset > 0) {
      return RemoteIterator(remote_id, offset, sizeof(uint64_t));
    }

    return RemoteIterator(remote_id,
                          LogConfig::round_up_powerof2(sizeof(LogHeader)),
                          sizeof(uint64_t));
  }

  std::vector<uint8_t> dump() const;
  
 private:
  size_t initial_fuo;
  size_t initial_free_bytes;
  uint8_t* buf;
  size_t len;
  LogHeader* header;
  std::array<std::pair<ptrdiff_t, size_t>, 3> offsets;
};

class Slot {
 public:
  Slot(Log& log) : log{log}, seq{0} { entry = log.newEntry(); }

  Slot(Log& log, uint64_t proposal_nr, uint64_t fuo, uint8_t* buf,
       size_t buf_len)
      : log{log} {
    entry = log.newEntry();
    entry.fast_store(proposal_nr, fuo, buf, buf_len);
    log.finalizeEntry(entry);
  }

  inline void storeAcceptedProposal(uint64_t proposal) {
    check_sequence(0);
    entry.store_uint64(proposal);
  }

  inline void storeFirstUndecidedOffset(uint64_t fuo) {
    check_sequence(1);
    entry.store_uint64(fuo);
  }

  inline void storePayload(const uint8_t* buf, size_t len) {
    check_sequence(2);
    entry.store_buf(buf, len);
    log.finalizeEntry(entry);
  }

  inline ParsedSlot toParsedSlot() const { return ParsedSlot(entry.basePtr()); }

  std::tuple<uint8_t*, ptrdiff_t, size_t> location() const {
    return std::make_tuple(entry.basePtr(), entry.basePtr() - log.headerPtr(),
                           entry.length());
  }

 private:
  inline void check_sequence(int x) {
    if (x != seq) {
      throw std::runtime_error(
          "Slot construction does not follow the proper order");
    }
    seq += 1;
  }

 private:
  Log& log;
  Log::Entry entry;
  int seq;
};


class Call {
 public:
  Call(HamsazLog& log, uint64_t log_offset, int* deps, uint8_t* buf, size_t buf_len, bool override)
      : log{log}, log_offset{log_offset}  {
    call = log.newCallEntry(log_offset, override);
    // no dependency condition
    // std::cout << "dep size " << sizeof(deps)/sizeof(deps[0]) << std::endl;
    // std::cout << "dep zero " << deps[0] << std::endl;
    
    if(deps == NULL){
      call.indicate_no_deps();
      // std::cout << "no dp" << std::endl;
    }
    else
      call.store_deps(deps);
    call.store_buf(buf, buf_len);
    log.finalizeCallEntry(call);
  }

  // inline void storeDependencies(int* deps) {
  //   // check_sequence(0);
  //   call.store_deps(deps);
  // }

  inline void storePayload(const uint8_t* buf, size_t len) {
    // check_sequence(2);
    call.store_buf(buf, len);
    log.finalizeCallEntry(call);
  }

  inline ParsedCall toParsedCall() const { return ParsedCall(call.basePtr()); }

  std::tuple<uint8_t*, ptrdiff_t, size_t> location() const {
    return std::make_tuple(call.basePtr(), call.basePtr() - log.bufPtr(),
                           call.length());
  }

 private:
  inline void check_sequence(int x) {
    if (x != seq) {
      throw std::runtime_error(
          "Slot construction does not follow the proper order");
    }
    seq += 1;
  }

 private:
  HamsazLog& log;
  uint64_t log_offset;
  HamsazLog::CallEntry call;
  int seq;
};

}  // namespace dory