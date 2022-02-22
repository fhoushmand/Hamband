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

#include <unordered_set>
#include <atomic>
#include "logger.hpp"
#include "replicated_object.hpp"

using namespace dory;

using dory::units::operator""_GiB;

int alignment = 64;
uint64_t fast_id;

namespace band {
class ReliableBroadcast {
 private:
  std::unique_ptr<ControlBlock> cb;
  std::vector<struct ibv_wc> entries;
  static constexpr int history_length = 50;
  Devices d;
  OpenDevice od;

  struct ReadingStatus {
    ReadingStatus()
        : value{0},
          consecutive_updates{0},
          failed_attempts{0},
          loop_modulo{0},
          freshly_updated{false} {}

    int outstanding;
    uint64_t value;
    int consecutive_updates;
    int failed_attempts;
    int loop_modulo;
    bool freshly_updated;
  };

 public:
  ReplicatedObject* replicated_object;
  std::vector<int> remote_ids;
  std::vector<int> ids;
  PartitionedIterator partIter;
  std::unique_ptr<ConnectionExchanger> ce;
  std::unique_ptr<BandLog> log;

  uint64_t post_id;
  std::vector<uint64_t> post_ids;
  std::vector<ReadingStatus> status;
  ptrdiff_t counter_offset;
  std::vector<uint8_t *> fd_counters;
  int read_seq = 0;
  std::unordered_set<int> failed_nodes;
  std::atomic<bool> hb_active;
  uint64_t *counter_from;
  int max_id;

  ReliableBroadcast(int id, std::vector<int> r_ids, ReplicatedObject* ReplicatedObject);
  bool broadcast(uint8_t *payload, size_t len, bool override);
  int pollCQ(int number);
  bool consume();
  void initPoller();
  void scanHeartbeats();
  void deliverAndExecute();
};


ReliableBroadcast::ReliableBroadcast(int id, std::vector<int> r_ids, ReplicatedObject* obj) {
  replicated_object = obj;
  remote_ids = r_ids;
  entries.resize(remote_ids.size());
  ids.insert(ids.end(), remote_ids.begin(), remote_ids.end());
  ids.push_back(replicated_object->self);
  std::sort(ids.begin(), ids.end());
  max_id = *(std::minmax_element(ids.begin(), ids.end()).second);

  // Exchange info using memcached
  auto &store = MemoryStore::getInstance();
  // Get the last device
  {
    for (auto &dev : d.list()) {
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
  cb->registerPD("band-primary");
  cb->allocateBuffer("band-shared-buf", LogConfig::LOG_SIZE, alignment);
  cb->registerMR("band-shared-mr", "band-primary", "band-shared-buf",
                 ControlBlock::LOCAL_READ | ControlBlock::LOCAL_WRITE |
                     ControlBlock::REMOTE_READ | ControlBlock::REMOTE_WRITE);
  cb->registerCQ("band-cq");
  ce = std::make_unique<ConnectionExchanger>(replicated_object->self, remote_ids, *cb.get());
  ce->configure_all("band-primary", "band-shared-mr", "band-cq",
                    "band-cq");
  ce->announce_all(store, "band-qp");
  ce->announce_ready(store, "band-qp", "announce");
  ce->wait_ready_all(store, "band-qp", "announce");

  std::this_thread::sleep_for(std::chrono::seconds(5));

  ce->connect_all(store, "band-qp",
                  ControlBlock::LOCAL_READ | ControlBlock::LOCAL_WRITE |
                      ControlBlock::REMOTE_READ | ControlBlock::REMOTE_WRITE);
  ce->announce_ready(store, "band-qp", "connect");

  auto shared_memory_addr =
      reinterpret_cast<uint8_t *>(cb->mr("band-shared-mr").addr);
  auto logmem = shared_memory_addr;
  memset(logmem + 8 * 64, 0x0, 512);
  auto logmem_size = LogConfig::LOG_SIZE;
  log = std::make_unique<BandLog>(logmem, logmem_size, replicated_object->num_process);

  partIter = log->partitionedIterator();

  counter_offset = replicated_object->self * sizeof(uint64_t);
  counter_from = reinterpret_cast<uint64_t *>(log.get()->bufPtr() + counter_offset);
  *counter_from = 0;

  fd_counters.push_back(log.get()->bufPtr());
  for (size_t i = 1; i <= replicated_object->num_process; i++)
    fd_counters.push_back(log.get()->bufPtr() + (i * sizeof(uint64_t)));

  status = std::vector<ReadingStatus>(max_id + 1);
  status[ids[0]].consecutive_updates = history_length;

  post_id = 0;
  post_ids.resize(max_id + 1);
  for (auto &id : post_ids) {
    id = post_id;
  }

  hb_active = true;

  // std::thread heartbeat_thd = std::thread([&]() {
  //   while (hb_active.load()) 
  //     scanHeartbeats();
  // });
  // std::thread fd_thd = std::thread([&]() {
  //   std::this_thread::sleep_for(std::chrono::seconds(3));
  //   while (true) {
  //     for (int i = 1; i <= static_cast<int>(ids.size()); i++) {
  //       if (i == replicated_object->self || failed_nodes.find(i) != failed_nodes.end()) continue;
  //       if (status[i].consecutive_updates <= 20){ 
  //         std::cout << "node " << i << " crashed" << std::endl;
  //         failed_nodes.insert(i);

  //         // check if there exists any unreceived writes
  //         // std::cout << "reading from " << i << " at: " << std::hex << ce->connections().at(i).remoteBuf() << ":" << std::dec << (8 * 64) << std::endl;

  //         std::vector<uint8_t> payload_buffer(256);
  //         uint8_t* payload = &payload_buffer[0];
  //         Call call(*log.get(), partIter.offset(replicated_object->self - 1),
  //                     payload, 256, false);
  //         auto [address, offset, size] = call.location();

  //         auto out = ce->connections().at(i).postSendSingle(
  //                 ReliableConnection::RdmaRead,
  //                 quorum::pack(quorum::EntryRd, i, 10),
  //                 address,
  //                 256, 
  //                 ce->connections().at(i).remoteBuf() + (8 * 64));
  //         std::this_thread::sleep_for(std::chrono::milliseconds(50));
  //         pollCQ(1);
  //         ParsedCall pslot = ParsedCall(address);
  //         auto [buf, len] = pslot.payload();
  //         // check if already applied it or not
  //         if (pslot.isPopulated()){
  //           MethodCall* c = replicated_object->deserialize(buf);
  //           // std::cout << "saved call of the crashed node: " << std::endl;
  //           // replicated_object.toString(*c);

  //           MethodCall* tailCall = NULL;
  //           while (partIter.hasNext(i - 1)) {
  //             ParsedCall pslot = ParsedCall(partIter.location(i - 1));
  //             auto [buf, len] = pslot.payload();
  //             if (!pslot.isPopulated()) break;
  //             tailCall = replicated_object->deserialize(buf);
  //             partIter.next(i - 1);
  //           }
  //           if(tailCall != NULL){
  //             // std::cout << "last call received from the crashed node: " << std::endl;  
  //             // replicated_object.toString(*tailCall);
  //             if(tailCall->id != c->id){
  //               std::cout << "fetching unreceived call from the crashed node (not-null)" << std::endl;
  //               replicated_object->internalExecute(*c, i - 1);
  //               replicated_object->printCall(*c);
  //             }
  //           }
  //           else{
  //             std::cout << "fetching unreceived call from the crashed node (null)" << std::endl;
  //             replicated_object->internalExecute(*c, i - 1);
  //             replicated_object->printCall(*c);
  //           }
  //         }
  //       }
  //     }
  //     std::this_thread::sleep_for(std::chrono::seconds(1));
  //   }
  // });

  std::thread nonConflictingLogReader([&] {
      while (true) {
        deliverAndExecute();
      std::this_thread::sleep_for(std::chrono::microseconds(400));
      }
      return;
    });
  nonConflictingLogReader.detach();

  // heartbeat_thd.detach();
  // fd_thd.detach();
}

void ReliableBroadcast::deliverAndExecute() {
  for (auto& i : remote_ids) 
  {
    while (partIter.hasNext(i - 1)) {
      ParsedCall pslot = ParsedCall(partIter.location(i - 1));
      auto [buf, len] = pslot.payload();
      if (!pslot.isPopulated()) break;
      MethodCall* call = replicated_object->deserialize(buf);
      // replicated_object->printCall(*call);
      replicated_object->internalExecute(*call, i - 1);
      partIter.next(i - 1);
    }
  }
  return;
}

void ReliableBroadcast::scanHeartbeats() {
  *counter_from += 1;
  memcpy(log.get()->bufPtr() + counter_offset, counter_from, sizeof(uint64_t));

  bool did_work = false;
  for (auto &[pid, rc] : ce->connections()) {
    did_work = true;
    post_ids[pid] = post_id;
    // reading from pid
    auto post_ret = rc.postSendSingle(
        ReliableConnection::RdmaRead,
        quorum::pack(quorum::LeaderHeartbeat, pid, read_seq), fd_counters[pid],
        sizeof(uint64_t), rc.remoteBuf() + (pid * sizeof(uint64_t)));
    if (!post_ret) {
      std::cout << "Post returned " << post_ret << std::endl;
    }
  }
  if (did_work) {
    post_id += 1;
  }
  read_seq++;

  entries.resize(2);
  int num = ibv_poll_cq(cb->cq("band-cq").get(), 10, &entries[0]);
  if (num > 0) {
    for (auto const &entry : entries) {
      auto [k, pid, seq] = quorum::unpackAll<int, uint64_t>(entry.wr_id);
      if (k == quorum::LeaderHeartbeat) {
        auto proc_post_id = post_ids[pid];
        volatile uint64_t *val = reinterpret_cast<uint64_t *>(fd_counters[pid]);
        // std::cout << "counter[" << pid << "]: " << *val << std::endl;
        if (pid == replicated_object->self) {
          val = reinterpret_cast<uint64_t *>(log.get()->bufPtr() + counter_offset);
        }

        if (status[pid].value == *val) {
          status[pid].consecutive_updates =
              std::max(status[pid].consecutive_updates, 1) - 1;
        } else {
          if (post_id < proc_post_id + 3) {
            status[pid].consecutive_updates =
                std::min(status[pid].consecutive_updates, history_length - 3) +
                3;
          }
        }
        status[pid].value = *val;
      }
    }
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


inline uint64_t fetchAndIncFastID() {
  auto ret = fast_id;
  fast_id += 1;
  return ret;
}

bool ReliableBroadcast::broadcast(uint8_t *payload, size_t length,
                                 bool summarize) {
  // std::cout << "INDEX " << replicated_object->self - 1 << std::endl;
  // for (size_t i = 0; i < replicated_object->num_process; i++) {
  //   std::cout << "offset " << partIter.offset(i) << std::endl;
  // }
  Call call(*log.get(), partIter.offset(replicated_object->self - 1), payload, length,
            summarize);
  // std::cout << "start broadcast1" << std::endl;
  auto [address, offset, size] = call.location();
  // std::cout << "start broadcast2" << std::endl;
  auto req_id = fetchAndIncFastID();
  // std::cout << "start broadcast3" << std::endl;


  // writing the call to the temporary location so others can read in case of
  // failure (backup)
  // std::cout << "writing locally for backup" << std::endl;
  memcpy(log.get()->bufPtr() + (8 * 64), address,
             static_cast<uint32_t>(size));
  // std::cout << "start broadcast4" << std::endl;
  

  int i = 1;
  for (auto &[pid, rc] : ce->connections()) {
    // if(replicated_object->self == 2)
      // std::cout << "writing " << payload << " to: " << pid << std::endl;
    // failure
    //   if(replicated_object->self == 1 && req_id == 1){
    //     hb_active.store(false);
    //     std::this_thread::sleep_for(std::chrono::seconds(60));
    // }

    auto ok = rc.postSendSingle(
        ReliableConnection::RdmaWrite,
        quorum::pack(quorum::EntryWr, i, req_id), address,
        static_cast<uint32_t>(size), rc.remoteBuf() + offset);
    
    i++;
  }
  pollCQ(static_cast<int>(remote_ids.size()));
  // clearing the backup
  // std::cout << "clearing backup location" << std::endl;
  memset(log.get()->bufPtr() + 8 * 64, 0x0, 512);
  // std::this_thread::sleep_for(std::chrono::seconds(4));
  return true;
}

bool ReliableBroadcast::consume() {
  auto ret = true;
  for (auto const &entry : entries) {
    if (entry.status != IBV_WC_SUCCESS) {
      std::cout << "write not successfull" << std::endl;
      ret = false;
    }
  }
  return ret;
}

int ReliableBroadcast::pollCQ(int number) {
  entries.resize(number);
  int num = ibv_poll_cq(cb->cq("band-cq").get(), number, &entries[0]);
  return 1;
}
}  // namespace band