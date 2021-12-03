#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <dory/conn/context.hpp>
#include <dory/conn/exchanger.hpp>
#include <dory/conn/rc.hpp>
#include <dory/ctrl/block.hpp>
#include <dory/ctrl/device.hpp>
#include <dory/log/log.hpp>
#include <dory/shared/message-identifier.hpp>
#include <dory/shared/units.hpp>
#include <dory/shared/unused-suppressor.hpp>
#include <dory/store.hpp>

#include <dory/log/context.hpp>
#include <dory/mem/memory.hpp>
#include <atomic>

#include "logger.hpp"

using namespace dory;

using dory::units::operator""_GiB;

// size_t allocated_size = 1_GiB;
int alignment = 64;

namespace hamsaz {
class BE_Broadcast {
 private:
  bool broadcastCall(uint8_t* payload, size_t len, uint64_t seq_number,
                     int* deps, bool override);
  std::unique_ptr<ControlBlock> cb;
  std::vector<uintptr_t> to_remote_memory, dest;
  std::vector<struct ibv_wc> entries;
  Devices d;
  OpenDevice od;
  

 public:
  int self;
  std::atomic<int> num_writes = 0;
  size_t num_process;
  std::vector<int> remote_ids;
  std::vector<int> ids;
  PartitionedIterator partIter;
  std::vector<ptrdiff_t> remote_offsets;
  std::unique_ptr<ConnectionExchanger> ce;
  std::unique_ptr<HamsazLog> log;
  BE_Broadcast(int id, std::vector<int> remote_ids);

  bool broadcastCallWithDep(uint8_t* payload, size_t len, uint64_t seq_number,
                            int* deps, bool override);
  bool broadcastCallNoDep(uint8_t* payload, size_t len, uint64_t seq_number,
                          bool override);

  bool sendCall(int receiver, uint8_t* payload, size_t len, uint64_t seq_number,
                            int* deps, bool override);

  bool pollCQ();

  bool consume(std::vector<struct ibv_wc>& entries);
  
  
};

BE_Broadcast::BE_Broadcast(int id, std::vector<int> r_ids) {
  self = id;
  num_process = r_ids.size() + 1;
  remote_ids = r_ids;
  ids.insert(ids.end(), remote_ids.begin(), remote_ids.end());
  ids.push_back(self);
  // Exchange info using memcached
  auto& store = MemoryStore::getInstance();
  // MemoryStore store = MemoryStore("beb");
  // Get the last device
  {
    // TODO: The copy constructor is invoked here if we use auto and then
    // iterate on the dev_lst
    for (auto& dev : d.list()) {
      od = std::move(dev);
    }
  }
  std::cout << od.name() << " " << od.dev_name() << " "
            << OpenDevice::type_str(od.node_type()) << " "
            << OpenDevice::type_str(od.transport_type()) << std::endl;

  ResolvedPort rp = ResolvedPort(od);
  auto binded = rp.bindTo(0);
  // Configure the control block
  cb = std::make_unique<ControlBlock>(rp);
  cb->registerPD("hamsaz-primary");
  cb->allocateBuffer("hamsaz-shared-buf", LogConfig::LOG_SIZE, alignment);
  cb->registerMR("hamsaz-shared-mr", "hamsaz-primary", "hamsaz-shared-buf",
                 ControlBlock::LOCAL_READ | ControlBlock::LOCAL_WRITE |
                     ControlBlock::REMOTE_READ | ControlBlock::REMOTE_WRITE);
  cb->registerCQ("hamsaz-cq");
  ce = std::make_unique<ConnectionExchanger>(self, remote_ids, *cb.get());
  ce->configure_all("hamsaz-primary", "hamsaz-shared-mr", "hamsaz-cq", "hamsaz-cq");
  ce->announce_all(store, "hamsaz-qp");
  ce->announce_ready(store, "hamsaz-qp", "announce");
  ce->wait_ready_all(store, "hamsaz-qp", "announce");

  std::this_thread::sleep_for(std::chrono::seconds(5));

  ce->connect_all(store, "hamsaz-qp",
                  ControlBlock::LOCAL_READ | ControlBlock::LOCAL_WRITE |
                      ControlBlock::REMOTE_READ | ControlBlock::REMOTE_WRITE);
  ce->announce_ready(store, "hamsaz-qp", "connect");

  auto shared_memory_addr =
      reinterpret_cast<uint8_t*>(cb->mr("hamsaz-shared-mr").addr);
  auto logmem = shared_memory_addr;
  auto logmem_size = LogConfig::LOG_SIZE;
  log = std::make_unique<HamsazLog>(logmem, logmem_size, num_process);
  std::cout << "log created" << std::endl;
  std::cout << "logmem_size: " << logmem_size << std::endl;
  printf("log address: %p", logmem);
  std::cout << std::endl;

  remote_offsets.resize(num_process);
  // each partition has an offset of: (logmem_size/num_process)*part_num
  for (size_t i = 0; i < num_process; i++) {
    remote_offsets[i] =
        LogConfig::round_up_powerof2(logmem_size / num_process) * i;
  }
  partIter = log->partitionedIterator();

  // adding log_offset to all the remote entries and store it in dest vector
  // std::cout << "max ids: " << Identifiers::maxID(ids) << std::endl;
  to_remote_memory.resize(Identifiers::maxID(ids));
  std::fill(to_remote_memory.begin(), to_remote_memory.end(),
            static_cast<uintptr_t>(0));
  dest = to_remote_memory;
}

bool BE_Broadcast::broadcastCall(uint8_t* payload, size_t len,
                                 uint64_t seq_number, int* deps,
                                 bool override = false) {
  // second argument indicates the offset for the corresponding log of this
  // process in the remote hosts
  Call call(*(log.get()), remote_offsets[self - 1], payload, len,
            override);
  auto [address, offset, size] = call.location();
  for (auto& c : ce->connections()) {
    // -1 is because ids are 1 based but arrays are zero based
    auto pid = c.first - 1;
    auto& rc = c.second;

    std::transform(to_remote_memory.begin(), to_remote_memory.end(),
                   dest.begin(), bind2nd(std::plus<uintptr_t>(), offset));

    if (false) {
      std::cout << "call offset: " << offset << std::endl;
      std::cout << "dest[pid]: " << dest[pid] << std::endl;
      // writing to remote
      std::cout << "writing " << payload << " to: " << pid + 1
                << " at: " << std::hex << rc.remoteBuf() + dest[pid]
                << std::endl;
      std::cout << "remote buf address " << std::hex << rc.remoteBuf()
                << std::endl;
      std::cout << std::endl << std::endl;
    }

    std::cout << "writing " << payload << " to: " << pid + 1
                << " at: " << std::hex << rc.remoteBuf() + dest[pid]
                << std::endl;
      

    auto ok = rc.postSendSingle(ReliableConnection::RdmaWrite,
                                quorum::pack(quorum::EntryWr, pid, seq_number),
                                address, static_cast<uint32_t>(size),
                                rc.remoteBuf() + dest[pid]);
    if (!ok) return false;
  }
  return true;
}

bool BE_Broadcast::broadcastCallWithDep(uint8_t* payload, size_t len,
                                        uint64_t seq_number, int* deps,
                                        bool override = false) {
  return broadcastCall(payload, len, seq_number, deps, override);
}

bool BE_Broadcast::broadcastCallNoDep(uint8_t* payload, size_t len,
                                      uint64_t seq_number,
                                      bool override = false) {
  return broadcastCall(payload, len, seq_number, NULL, override);
}

bool BE_Broadcast::consume(std::vector<struct ibv_wc>& entries) {
  auto ret = true;
  for (auto const& entry : entries) {
    if (entry.status != IBV_WC_SUCCESS) {
      std::cout << "write not successfull" << std::endl;
      ret = false;
    } else {
      num_writes++;
      auto [k, pid, seq] = quorum::unpackAll<int64_t, int64_t>(entry.wr_id);

      // if (k != kind) {
      //   std::cout << "Mismatched message kind: (" << quorum::type_str(k)
      //             << " vs " << quorum::type_str(kind)
      //             << "). Received unexpected/old message response from the "
      //                "completion queue"
      //             << std::endl;
      //   continue;
      // }

      // auto current_seq = scoreboard[pid];
      // scoreboard[pid] = next_id - current_seq == modulo ? seq : 0;

      // if (scoreboard[pid] == next_id) {
      //   left -= 1;
      // }

      // if (left == 0) {
      //   left = quorum_size;
      //   next_id += modulo;
      // }
    }
  }
  return ret;
}

bool BE_Broadcast::pollCQ()
{
  entries.resize(remote_ids.size()*10);
  cb->pollCqIsOK(cb->cq("hamsaz-cq"), entries);
  return consume(entries);
}

}  // namespace hamsaz