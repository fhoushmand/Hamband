#include "memory.hpp"

using dory::units::operator""_MiB;



namespace dory {
OverlayAllocator::OverlayAllocator(void* buf, size_t size)
    : buf{reinterpret_cast<uint8_t*>(buf)}, size{size},
    LOGGER_INIT(logger, "") {}

std::pair<bool, uint8_t*> OverlayAllocator::allocate(size_t length,
                                                     size_t alignment) {
  // LOGGER_INFO(logger, "allocating {} with {} alignment", length, alignment);
  if (remaining() < length) {
    return std::make_pair(false, nullptr);
  }

  auto next = next_aligned(alignment);
  
  if (next + length > buf + size) {
    return std::make_pair(false, nullptr);
  }
  
  allocations.push_back(std::make_pair(next, length));
  
  return std::make_pair(true, next);
}

std::tuple<bool, uint8_t*, size_t> OverlayAllocator::allocateRemaining(
    size_t alignment) {
  auto next = next_aligned(alignment);
  auto len = buf + size - next;
  auto [ok, ptr] = allocate(len, alignment);
  // LOGGER_INFO(logger, "{} size: {}, next: 0x{:x}, ptr: ox{:x}, len: {}, buf: 0x{:x}", ok ? "allocated" : "not allocated", size, uintptr_t(next), uintptr_t(ptr), len, uintptr_t(buf));
  return std::make_tuple(ok, ptr, len);
}

size_t OverlayAllocator::remaining() {
  // Get the last allocation
  auto sz = allocations.size();
  // LOGGER_INFO(logger, "call to remaining -- allocations size: {}, last allocation-> ptr: 0x{:x}, size: {}", sz, uintptr_t(allocations[sz - 1].first), allocations[sz - 1].second);
  if (sz > 0) {
    return buf - allocations[sz - 1].first + size - allocations[sz - 1].second;
  } else {
    return size;
  }
}

uint8_t* OverlayAllocator::next_aligned(size_t alignment) {
  auto unaligned = buf + size - remaining();
  if (uintptr_t(unaligned) % alignment != 0) {
    return unaligned - uintptr_t(unaligned) % alignment + alignment;
  } else {
    return unaligned;
  }
}
}  // namespace dory

namespace dory {
ScratchpadMemory::ScratchpadMemory(std::vector<int>& ids,
                                   OverlayAllocator& overlay, int alignment)
    : max_id{*std::max_element(ids.begin(), ids.end())},
      base{reinterpret_cast<uint8_t*>(overlay.base())},
      LOGGER_INIT(logger, "") {
  // Get the memory requirment
  Memory memory(alignment);
  ScratchpadMemory scratchpad(ids, memory);
  auto scratchpad_required_mem = scratchpad.requiredSize();
  // 1024KB = 1MB = slotSize()
  // scratchpad_required_mem = 33MB
  LOGGER_INFO(logger,
              "Scratchpad memory :: slot size: {} bytes, total size: {} bytes.",
              slotSize(), scratchpad_required_mem);

  // Do the allocation
  auto [scratchpad_ok, scratchpad_mem] =
      overlay.allocate(scratchpad_required_mem, alignment);
  if (!scratchpad_ok) {
    throw std::runtime_error("Overlay allocation exceeded");
  }

  memset(scratchpad_mem, 0, scratchpad_required_mem);

  mem = Memory(scratchpad_mem, scratchpad_required_mem, alignment);
  next = mem.ptr;

  setup();

  if (next > mem.ptr + mem.size) {
    throw std::runtime_error("Memory allocation exceeded");
  }
}

ScratchpadMemory::ScratchpadMemory(std::vector<int>& ids, Memory const& mem)
    : max_id{*std::max_element(ids.begin(), ids.end())}, mem{mem}, next{mem.ptr}, base{nullptr} {
  setup();
}

void ScratchpadMemory::setup() {
  setupReadFUOSlots();
  setupReadProposalNrSlots();
  setupReadLogEntrySlots();
  setupReadLeaderChangeSlots();
  setupWriteLeaderChangeSlots();
  setupReadLogRecyclingSlots();
  setupLogRecyclingResponseSlot();
  setupWriteSlot();
  setupLeaderRequestSlot();
  setupLeaderResponseSlot();
  setupLeaderHeartbeatSlot();
  setupReadLeaderHeartbeatSlots();
}

size_t ScratchpadMemory::requiredSize() const { 
  std::cout << "memory size: " << next - mem.ptr << std::endl;
  return next - mem.ptr; 
}

inline size_t ScratchpadMemory::slotSize() const {
  return 1_MiB;
}

std::vector<uint8_t*>& ScratchpadMemory::readFUOSlots() {
  return read_fuo_slots;
}

std::vector<ptrdiff_t>& ScratchpadMemory::readFUOSlotsOffsets() {
  return read_fuo_slots_offsets;
}

std::vector<uint8_t*>& ScratchpadMemory::readProposalNrSlots() {
  return read_proposal_nr_slots;
}

std::vector<ptrdiff_t>& ScratchpadMemory::readProposalNrSlotsOffsets() {
  return read_proposal_nr_slots_offsets;
}

std::vector<uint8_t*>& ScratchpadMemory::readLogEntrySlots() {
  return read_log_entry_slots;
}

std::vector<ptrdiff_t>& ScratchpadMemory::readLogEntrySlotsOffsets() {
  return read_log_entry_slots_offsets;
}

std::vector<uint8_t*>& ScratchpadMemory::readLeaderChangeSlots() {
  return read_leader_change_slots;
}

std::vector<ptrdiff_t>& ScratchpadMemory::readLeaderChangeSlotsOffsets() {
  return read_leader_change_slots_offsets;
}

std::vector<uint8_t*>& ScratchpadMemory::writeLeaderChangeSlots() {
  return write_leader_change_slots;
}

std::vector<ptrdiff_t>& ScratchpadMemory::writeLeaderChangeSlotsOffsets() {
  return write_leader_change_slots_offsets;
}

std::vector<uint8_t*>& ScratchpadMemory::readLogRecyclingSlots() {
  return read_log_recycling_slots;
}

std::vector<ptrdiff_t>& ScratchpadMemory::readLogRecyclingSlotsOffsets() {
  return read_log_recycling_slots_offsets;
}

uint8_t* ScratchpadMemory::logRecyclingResponseSlot() {
  return log_recycling_response_slot;
}

uint8_t* ScratchpadMemory::writeSlot() { return write_slot; }

ptrdiff_t ScratchpadMemory::writeSlotOffset() { return write_slot_offset; }

uint8_t* ScratchpadMemory::leaderRequestSlot() { return leader_req_slot; }

ptrdiff_t ScratchpadMemory::leaderRequestSlotOffset() {
  return leader_req_slot_offset;
}

uint8_t* ScratchpadMemory::leaderResponseSlot() { return leader_resp_slot; }

ptrdiff_t ScratchpadMemory::leaderResponseSlotOffset() {
  return leader_resp_slot_offset;
}

uint8_t* ScratchpadMemory::leaderHeartbeatSlot() {
  return leader_heartbeat_slot;
}

ptrdiff_t ScratchpadMemory::leaderHeartbeatSlotOffset() {
  return leader_heartbeat_slot_offset;
}

std::vector<uint8_t*>& ScratchpadMemory::readLeaderHeartbeatSlots() {
  return read_leader_heartbeat_slots;
}

std::vector<ptrdiff_t>& ScratchpadMemory::readLeaderHeartbeatSlotsOffsets() {
  return read_leader_heartbeat_slots_offsets;
}

void ScratchpadMemory::setupReadFUOSlots() {
  setupSlots(read_fuo_slots, read_fuo_slots_offsets);
}

void ScratchpadMemory::setupReadProposalNrSlots() {
  setupSlots(read_proposal_nr_slots, read_proposal_nr_slots_offsets);
}

void ScratchpadMemory::setupReadLogEntrySlots() {
  setupSlots(read_log_entry_slots, read_log_entry_slots_offsets);
}

void ScratchpadMemory::setupReadLeaderChangeSlots() {
  setupSlots(read_leader_change_slots, read_leader_change_slots_offsets);
}

void ScratchpadMemory::setupWriteLeaderChangeSlots() {
  setupSlots(write_leader_change_slots, write_leader_change_slots_offsets);
}

void ScratchpadMemory::setupReadLogRecyclingSlots() {
  setupSlots(read_log_recycling_slots, read_log_recycling_slots_offsets);
}

void ScratchpadMemory::setupLogRecyclingResponseSlot() {
  setupSlot(log_recycling_response_slot, log_recycling_response_slot_offset);
}

void ScratchpadMemory::setupWriteSlot() {
  setupSlot(write_slot, write_slot_offset);
}

void ScratchpadMemory::setupLeaderRequestSlot() {
  setupSlot(leader_req_slot, leader_req_slot_offset);
}

void ScratchpadMemory::setupLeaderResponseSlot() {
  setupSlot(leader_resp_slot, leader_resp_slot_offset);
}

void ScratchpadMemory::setupLeaderHeartbeatSlot() {
  setupSlot(leader_heartbeat_slot, leader_heartbeat_slot_offset);
}

void ScratchpadMemory::setupReadLeaderHeartbeatSlots() {
  setupSlots(read_leader_heartbeat_slots, read_leader_heartbeat_slots_offsets);
}

void ScratchpadMemory::setupSlots(std::vector<uint8_t*>& slots,
                                  std::vector<ptrdiff_t>& offsets) {
  slots.resize(max_id + 1);
  offsets.resize(max_id + 1);

  for (int pid = 0; pid <= max_id; pid++) {
    slots[pid] = next + pid * slotSize();
    offsets[pid] = slots[pid] - base;
  }

  next += slots.size() * slotSize();
  next = align_up(next);
}

void ScratchpadMemory::setupSlot(uint8_t*& slot, ptrdiff_t& offset) {
  slot = next;
  offset = slot - base;

  next += slotSize();
  next = align_up(next);
}

uint8_t* ScratchpadMemory::align_up(uint8_t* ptr) {
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

  auto boundary = mem.alignment;
  if (addr % boundary != 0) {
    addr += boundary - addr % boundary;
  }

  return reinterpret_cast<uint8_t*>(addr);
}
}  // namespace dory



// ADDED by FARZIN
namespace dory {
HamsazMemory::HamsazMemory(std::vector<int>& ids,
                                   OverlayAllocator& overlay, int alignment)
    : max_id{*std::max_element(ids.begin(), ids.end())},
      base{reinterpret_cast<uint8_t*>(overlay.base())},
      LOGGER_INIT(logger, "") {
  // Get the memory requirment
  Memory memory(alignment);
  HamsazMemory scratchpad(ids, memory);
  auto scratchpad_required_mem = scratchpad.requiredSize();
  // 1024KB = 1MB = slotSize()
  // scratchpad_required_mem = 33MB
  LOGGER_INFO(logger,
              "Hamsaz memory :: slot size: {} bytes, total size: {} bytes.",
              slotSize(), scratchpad_required_mem);

  // Do the allocation
  auto [scratchpad_ok, scratchpad_mem] =
      overlay.allocate(scratchpad_required_mem, alignment);
  if (!scratchpad_ok) {
    throw std::runtime_error("Overlay allocation exceeded");
  }

  memset(scratchpad_mem, 0, scratchpad_required_mem);

  mem = Memory(scratchpad_mem, scratchpad_required_mem, alignment);
  next = mem.ptr;

  setup();

  if (next > mem.ptr + mem.size) {
    throw std::runtime_error("Memory allocation exceeded");
  }
}

HamsazMemory::HamsazMemory(std::vector<int>& ids, Memory const& mem)
    : max_id{*std::max_element(ids.begin(), ids.end())}, mem{mem}, next{mem.ptr}, base{nullptr} {
  setup();
}

void HamsazMemory::setup() {
  setupDepositSlot();
}

size_t HamsazMemory::requiredSize() const { 
  // std::cout << "memory size: " << next - mem.ptr << std::endl;
  return next - mem.ptr; 
}

inline size_t HamsazMemory::slotSize() const {
  return 1_MiB;
}

uint8_t* HamsazMemory::depositSlot() { return deposit_slot; }

void HamsazMemory::setupDepositSlot() {
  setupSlot(deposit_slot, deposit_slot_offset);
}


void HamsazMemory::setupSlot(uint8_t*& slot, ptrdiff_t& offset) {
  slot = next;
  offset = slot - base;

  next += slotSize();
  next = align_up(next);
}

uint8_t* HamsazMemory::align_up(uint8_t* ptr) {
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

  auto boundary = mem.alignment;
  if (addr % boundary != 0) {
    addr += boundary - addr % boundary;
  }

  return reinterpret_cast<uint8_t*>(addr);
}


}