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

#include <atomic>
#include <dory/log/context.hpp>
#include <dory/mem/memory.hpp>

#include "logger.hpp"

using namespace dory;

using dory::units::operator""_GiB;

int alignment = 64;
uint64_t next_id;
uint64_t fast_id;

namespace hamsaz {
class BE_Broadcast {
 private:
  std::unique_ptr<ControlBlock> cb;
  std::vector<struct ibv_wc> entries;
  Devices d;
  OpenDevice od;

 public:
  int self;
  size_t num_process;
  std::vector<int> remote_ids;
  std::vector<int> ids;
  std::vector<ptrdiff_t> partitions_offset;
  PartitionedIterator partIter;
  std::unique_ptr<ConnectionExchanger> ce;
  std::unique_ptr<HamsazLog> log;
  
  BE_Broadcast(int id, std::vector<int> remote_ids);
  bool broadcastCall(uint8_t* payload, size_t len, bool override);
  int pollCQ();
  bool consume();
};

BE_Broadcast::BE_Broadcast(int id, std::vector<int> r_ids) {
  self = id;
  num_process = r_ids.size() + 1;
  remote_ids = r_ids;
  entries.resize(remote_ids.size()*1000);
  partitions_offset.resize(num_process);
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
  ce->configure_all("hamsaz-primary", "hamsaz-shared-mr", "hamsaz-cq",
                    "hamsaz-cq");
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
  std::cout << "logmem_size: " << logmem_size << std::endl;
  printf("log address: %p", logmem);
  std::cout << std::endl;

  // each partition has an offset of:
  // (LogConfig::LOG_SIZE/num_process)*part_num
  for (size_t i = 0; i < num_process; i++) {
    partitions_offset[i] =
      LogConfig::round_up_powerof2(LogConfig::LOG_SIZE / num_process) * i + 1024;
  }

  partIter = log->partitionedIterator();
}

inline uint64_t fetchAndIncFastID() {
    auto ret = fast_id;
    fast_id += 1;
    return ret;
  }

inline uint64_t nextFastReqID() { return fast_id; }

bool BE_Broadcast::broadcastCall(uint8_t* payload, size_t length, bool summarize) {
  Call call(*log.get(), partitions_offset[self - 1],
                payload, length, summarize);
  auto [address, offset, size] = call.location();

  auto req_id = fetchAndIncFastID();
  auto next_req_id = nextFastReqID();

  for (auto& conn : ce->connections()) {
    std::cout << "writing " << payload << " to: " << conn.first << " at: " <<
    std::hex
              << conn.second.remoteBuf() << "\t"
              << offset
              << std::endl;
    auto ok = conn.second.postSendSingle(
        ReliableConnection::RdmaWrite, quorum::pack(quorum::EntryWr, self, req_id),
        address, static_cast<uint32_t>(size),
        conn.second.remoteBuf() + offset);
    if(conn.first == 1)
    {
      memcpy(log.get()->bufPtr(), address, 10);
    }

  }
  return true;
}

bool BE_Broadcast::consume() {
  auto ret = true;
  for (auto const& entry : entries) {
    if (entry.status != IBV_WC_SUCCESS) {
      std::cout << "write not successfull" << std::endl;
      ret = false;
    }
  }
  // entries.clear();
  return ret;
}

int BE_Broadcast::pollCQ() {
  // entries.resize(static_cast<int>(remote_ids.size()) * 8 + 3);
  // cb->pollCqIsOK(cb->cq("hamsaz-cq"), remote_ids.size() * 8 + 3);
  int num = ibv_poll_cq(cb->cq("hamsaz-cq").get(), static_cast<int>(remote_ids.size())*1000, &entries[0]);
  // std::cout << "num polled: " << num << std::endl; 
  // if (num >= 0) 
  //   consume();
  // ibv_poll_cq(cb->cq("hamsaz-cq").get(), static_cast<int>(remote_ids.size()),
              // &entries[0]);
  return 1;
  // return ibv_poll_cq(cb->cq("hamsaz-cq").get(),
  // static_cast<int>(entries.size()), &entries[0]);
}

}  // namespace hamsaz