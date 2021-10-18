#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <string>

#include <dory/conn/context.hpp>
#include <dory/conn/exchanger.hpp>
#include <dory/conn/rc.hpp>
#include <dory/ctrl/block.hpp>
#include <dory/ctrl/device.hpp>
#include <dory/log/log-slot-reader.hpp>
#include <dory/log/log.hpp>
#include <dory/log/remote-log-reader.hpp>
#include <dory/shared/message-identifier.hpp>
#include <dory/shared/units.hpp>
#include <dory/shared/unused-suppressor.hpp>
#include <dory/store.hpp>

#include <dory/log/context.hpp>
#include <dory/mem/memory.hpp>

#include "logger.hpp"

using namespace dory;

void mkrndstr_ipa(int length, uint8_t* randomString);

void mkrndstr_ipa(int length, uint8_t* randomString) {
  static uint8_t charset[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  if (length) {
    if (randomString) {
      int l = static_cast<int>(sizeof(charset) - 1);
      for (int n = 0; n < length; n++) {
        int key = rand() % l;
        randomString[n] = charset[key];
      }

      randomString[length] = '\0';
    }
  }
}

int main(int argc, char* argv[]) {
  LOGGER_DECL(logger);
  // LOGGER_INIT(logger, ConsensusConfig::logger_prefix);
  if (argc < 2) {
    throw std::runtime_error("Provide the id of the process as argument");
  }

  constexpr int nr_procs = 3;
  constexpr int minimum_id = 1;
  int id = 0;
  switch (argv[1][0]) {
    case '1':
      id = 1;
      break;
    case '2':
      id = 2;
      break;
    case '3':
      id = 3;
      break;
    case '4':
      id = 4;
      break;
    case '5':
      id = 5;
      break;
    default:
      throw std::runtime_error("Invalid id");
  }

  using namespace units;
  size_t allocated_size = 1_GiB;

  int alignment = 64;

  // Build the list of remote ids
  std::vector<int> remote_ids;
  for (int i = 0, min_id = minimum_id; i < nr_procs; i++, min_id++) {
    if (min_id == id) {
      continue;
    } else {
      remote_ids.push_back(min_id);
    }
  }

  std::vector<int> ids(remote_ids);
  ids.push_back(id);

  // Exchange info using memcached
  auto& store = MemoryStore::getInstance();

  Devices d;
  OpenDevice od;

  // Get the last device
  {
    // TODO: The copy constructor is invoked here if we use auto and then
    // iterate on the dev_lst
    // auto dev_lst = d.list();
    for (auto& dev : d.list()) {
      od = std::move(dev);
    }
  }

  std::cout << od.name() << " " << od.dev_name() << " "
            << OpenDevice::type_str(od.node_type()) << " "
            << OpenDevice::type_str(od.transport_type()) << std::endl;

  ResolvedPort rp(od);
  auto binded = rp.bindTo(0);
  std::cout << "Binded successful? " << binded << std::endl;
  std::cout << "(port_id, port_lid) = (" << +rp.portID() << ", "
            << +rp.portLID() << ")" << std::endl;

  // Configure the control block
  ControlBlock cb(rp);
  cb.registerPD("primary");
  cb.allocateBuffer("shared-buf", allocated_size, alignment);
  cb.registerMR("shared-mr", "primary", "shared-buf",
                ControlBlock::LOCAL_READ | ControlBlock::LOCAL_WRITE |
                    ControlBlock::REMOTE_READ | ControlBlock::REMOTE_WRITE);
  cb.registerCQ("cq");

  ConnectionExchanger ce(id, remote_ids, cb);
  ce.configure_all("primary", "shared-mr", "cq", "cq");
  ce.announce_all(store, "qp");
  ce.announce_ready(store, "qp", "announce");
  ce.wait_ready_all(store, "qp", "announce");


  std::cout << "Waiting (5 sec) for all processes to fetch the connections" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
    

  ce.connect_all(store, "qp",
                 ControlBlock::LOCAL_READ | ControlBlock::LOCAL_WRITE |
                     ControlBlock::REMOTE_READ | ControlBlock::REMOTE_WRITE);

  ce.announce_ready(store, "qp", "connect");

  auto& cq = cb.cq("cq");

  auto shared_memory_addr = reinterpret_cast<uint8_t*>(cb.mr("shared-mr").addr);

  ConnectionContext conn_ctx = ConnectionContext(cb, ce, cq, ids, id);

  // OverlayAllocator overlay = OverlayAllocator(shared_memory_addr, 1_GiB);
  // 14MB for the other slots
  // ScratchpadMemory scratchpad = ScratchpadMemory(ids, overlay, alignment);
  // 1010MB for the actual log
  // auto [logmem_ok, logmem, logmem_size] = overlay.allocateRemaining(64);
  auto logmem = shared_memory_addr;
  auto logmem_size = 1_GiB;
  auto log_offset = logmem - shared_memory_addr;  // should be zero
  std::cout << "log_offset: " << log_offset << std::endl;
  printf("logmem: %p", logmem);
  std::cout << std::endl;
  std::cout << "logmem_size: " << logmem_size << std::endl;
  Log replication_log = Log(logmem, logmem_size);

  // for multiple logs per replica
  // each iterator is for the log of the corresponding replica
  // for example: in replica 1, iter[0] is for it own log,
  // iter[1] is for the log that replica 2 manages and 
  // iter[2] is for the log that replica number 3 manages. 
  uintptr_t entry_num = 1_GiB / 64;
  uintptr_t entry_per_replica = entry_num / nr_procs;
  std::vector<uintptr_t> offsets;
  std::vector<BlockingIterator> iter;
  std::vector<LiveIterator> commit_iter;
  for(int i = 0; i < nr_procs; i++){
    offsets.push_back(entry_per_replica * i);
    // std::cout << "offset: " << std::hex << entry_per_replica * i << std::endl;
    // printf("new buf: %p",logmem + offsets[i]);
    std::cout << std::endl;
    iter.push_back(replication_log.blockingIterator(logmem + offsets[i]));
    commit_iter.push_back(replication_log.liveIterator(logmem + offsets[i]));
  }
  
  ReplicationContext repl_ctx = ReplicationContext(conn_ctx, replication_log, log_offset);

  std::vector<uint8_t> payload_buffer(8 + 2);
  uint8_t* payload = &payload_buffer[0];

  // adding log_offset to all the remote entries and store it in dest vector
  std::vector<uintptr_t> to_remote_memory;
  to_remote_memory.resize(Identifiers::maxID(ids) + 1);
  std::fill(to_remote_memory.begin(), to_remote_memory.end(), log_offset);
  std::vector<uintptr_t> dest = to_remote_memory;
  uintptr_t start = 0;
  
  auto fuo = -64;
  ptrdiff_t r_offset;
  uint32_t r_size;
  // if (id == 1) {
    for (int i = 0; i < 5; i++) {
      fuo += 64;
      // uint64_t *payload = reinterpret_cast<uint64_t
      // *>(scratchpad.writeSlot());
      if(id == 1)
        strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "1-");
      else if(id == 2)
        strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "2-");
      else 
        strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "3-");
      // mkrndstr_ipa(8, payload);
      // second argument indicates the offset for the corresponding log of this process in the remote hosts
      Call call(replication_log, offsets[id-1], fuo, static_cast<uint8_t*>(payload), 10);
      auto [address, offset, size] = call.location();
      // std::cout << "size: " << size << std::endl;
      // printf("slot addres: %p", address);
      // std::cout << std::endl;
      // std::cout << "slot offset: " << offset << std::endl;

      // ParsedSlot parsed = ParsedSlot(address);
      // std::cout << "parsed slot prop: " << parsed.acceptedProposal() << std::endl;
      // std::cout << "parsed slot fuo: " << parsed.firstUndecidedOffset() << std::endl;
      // std::cout << "parsed slot payload: " << parsed.payload().first << std::endl;
      // std::cout << "parsed slot length: " << parsed.totalLength() << std::endl;
 

      for (auto& c : ce.connections()) {
        auto pid = c.first;
        auto& rc = c.second;

        
        std::transform(to_remote_memory.begin(), to_remote_memory.end(),
                     dest.begin(), bind2nd(std::plus<uintptr_t>(), offset));

        // auto store_addr = scratchpad.readLogEntrySlots()[pid];
        // auto offset_size = remote_iterators[pid].iterator().lookAt(offset);
        // r_offset = offset_size.first;
        // r_size = static_cast<uint32_t>(offset_size.second);
        // remote_iterators[pid].iterator().storeDest(store_addr);

        // local write
        

        // writing to remote
        std::cout << "writing " << payload << " to: " << pid << " at: " << std::hex << rc.remoteBuf() + dest[pid];
        // auto sum = rc.remoteBuf() + dest[pid];
        // printf(" at: %zd", sum);
        std::cout << std::endl;
        // std::cout << "remote buffer " << pid << ": " << std::hex << rc.remoteBuf() << std::endl;
        // std::cout << "remote offset " << pid << ": " << dest[pid] << std::endl;
        auto ok = rc.postSendSingle(ReliableConnection::RdmaWrite,
                                    quorum::pack(quorum::EntryWr, pid, 1),
                                    address, static_cast<uint32_t>(size),
                                    rc.remoteBuf() + dest[pid]);
        if (!ok) std::cout << "error in write!!" << std::endl;
        std::cout << "-------" << std::endl;
      }
      std::cout << "========" << std::endl;
    // }
    std::cout << "finished writing, now waiting..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cout << "done waiting" << std::endl;
  }

  // reading (remote and local)
  for(int i = 0; i < nr_procs; i++){
    std::cout << "start reading " << i+1 << " log" << std::endl;  
    while (true) {
      auto has_next = iter[i].sampleNext();
      if (!has_next) {
        continue;
      }
      // std::cout << "blocked iterator" << std::endl;
      ParsedSlot pslot(iter[i].location());
      auto remote_fuo = pslot.firstUndecidedOffset();
      // auto has_next = iter.sampleNext();
      if (has_next) {
        // std::cout << "live iterator" << std::endl;
        // std::cout << "has entry in log\n";
        // ParsedSlot pslot(iter.location());
        // std::cout << "buffer: " << pslot.payload().first << std::endl;
        while (commit_iter[i].hasNext(100000)) {
          commit_iter[i].next();
          ParsedSlot pslot(commit_iter[i].location());
          auto [buf, len] = pslot.payload();
          if (!pslot.isPopulated()) continue;
          std::cout << "buffer: " << buf << std::endl;
          // break;
        }
        break;
      }
    }
  }
  std::cout << "end\n";
  return 0;
}














/*
  LogSlotReader lsr = LogSlotReader(&repl_ctx, scratchpad, start);
  auto resp = lsr.readSlotAt(static_cast<uint64_t>(64));
  std::cout << "read performed\n";
  if (resp->ok()) {
    auto& successes = lsr.successes();
    // ParsedSlot local_pslot(local_fuo_entry);
    // if (local_pslot.isPopulated()) {
    //   auto [buf, len] = local_pslot.payload();
    // }
    for (auto pid : successes) {
      if (pid < 0) {
        std::cout << "Nothing to read" << std::endl;
      } else {
        auto store_addr = scratchpad.readLogEntrySlots()[pid];
        ParsedSlot pslot(store_addr);
        auto [buf, len] = pslot.payload();
        std::cout << "read from " << pid << " successful with value " << buf
                  << std::endl;
      }
    }
  }
  */