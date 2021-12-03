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
using dory::units::operator""_GiB;

size_t allocated_size = 4_GiB;
int alignment = 64;


namespace hamsaz{
    class BE_Broadcast
    {
    private:
        uint64_t fuo = -64;
        std::vector<uintptr_t> to_remote_memory, dest;
        Devices d;
        OpenDevice od;
        std::unique_ptr<Log> log;
        std::unique_ptr<ConnectionExchanger> ce;
        std::unique_ptr<ControlBlock> cb;

    public:
        int self;
        size_t num_process;
        std::vector<int> remote_ids;
        std::vector<int> ids;
        std::vector<BlockingIterator> iter;
        std::vector<LiveIterator> commit_iter;
        std::vector<uintptr_t> remote_offsets;
        BE_Broadcast(int id, std::vector<int> remote_ids);
        ~BE_Broadcast();
        bool broadcastCall(uint8_t* payload, size_t len, int* deps, bool override);
        bool broadcastCallWithDep(uint8_t* payload, size_t len, int* deps, bool override);
        bool broadcastCallNoDep(uint8_t* payload, size_t len, bool override);
        // Log getLog();
    };
    
    BE_Broadcast::BE_Broadcast(int id, std::vector<int> r_ids)
    {
        self = id;
        num_process = r_ids.size() + 1;
        remote_ids = r_ids;
        ids.insert(ids.end(), remote_ids.begin(), remote_ids.end());
        ids.push_back(self);
        // Exchange info using memcached
        auto& store = MemoryStore::getInstance();
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
        cb->registerPD("primary");
        cb->allocateBuffer("shared-buf", allocated_size, alignment);
        cb->registerMR("shared-mr", "primary", "shared-buf",
                        ControlBlock::LOCAL_READ | ControlBlock::LOCAL_WRITE |
                            ControlBlock::REMOTE_READ | ControlBlock::REMOTE_WRITE);
        cb->registerCQ("cq");
        ce = std::make_unique<ConnectionExchanger>(self, remote_ids, *cb.get());
        ce->configure_all("primary", "shared-mr", "cq", "cq");
        ce->announce_all(store, "qp");
        ce->announce_ready(store, "qp", "announce");
        ce->wait_ready_all(store, "qp", "announce");
        
        ce->connect_all(store, "qp",
                        ControlBlock::LOCAL_READ | ControlBlock::LOCAL_WRITE |
                            ControlBlock::REMOTE_READ | ControlBlock::REMOTE_WRITE);
        ce->announce_ready(store, "qp", "connect");

        auto shared_memory_addr = reinterpret_cast<uint8_t*>(cb->mr("shared-mr").addr);
        auto logmem = shared_memory_addr;
        auto logmem_size = allocated_size;
        // printf("logmem: %p", logmem);
        // std::cout << std::endl;
        std::cout << "logmem_size: " << logmem_size << std::endl;
        log = std::make_unique<Log>(logmem, logmem_size);
        std::cout << "log created" <<  std::endl;
        
        // for multiple logs per replica
        // each iterator is for the log of the corresponding replica
        // for example: in replica 1, iter[0] is for it own log,
        // iter[1] is for the log that replica 2 manages and 
        // iter[2] is for the log that replica number 3 manages. 
        uintptr_t entry_num = allocated_size / 64;
        uintptr_t entry_per_replica = entry_num / num_process;
        for(size_t i = 0; i < num_process; i++){
            remote_offsets.push_back(entry_per_replica * i);
            // std::cout << "offset: " << std::hex << entry_per_replica * i << std::endl;
            // printf("new buf: %p",logmem + offsets[i]);
            // std::cout << std::endl;
            iter.push_back(log->blockingIterator(logmem + remote_offsets[i]));
            commit_iter.push_back(log->liveIterator(logmem + remote_offsets[i]));
        }

        // adding log_offset to all the remote entries and store it in dest vector
        to_remote_memory.resize(Identifiers::maxID(ids) + 1);
        std::fill(to_remote_memory.begin(), to_remote_memory.end(), 0);
        dest = to_remote_memory;

    }

    bool BE_Broadcast::broadcastCall(uint8_t* payload, size_t len, int* deps, bool override = false) {        
        if (override)
            fuo = 0;
        else
            fuo += 64;
        // second argument indicates the offset for the corresponding log of this
        // process in the remote hosts
        Call call(*(log.get()), remote_offsets[self - 1], deps, payload, len);
        auto [address, offset, size] = call.location();
        // std::cout << "size: " << size << std::endl;
        // printf("slot addres: %p", address);
        // std::cout << std::endl;
        // std::cout << "slot offset: " << offset << std::endl;
        
        for (auto& c : ce->connections()) {
            auto pid = c.first;
            auto& rc = c.second;

            std::transform(to_remote_memory.begin(), to_remote_memory.end(),
                            dest.begin(), bind2nd(std::plus<uintptr_t>(), offset));

            // writing to remote
            std::cout << "writing " << payload << " to: " << pid << " at: " << std::hex
                        << rc.remoteBuf() + dest[pid];
            auto ok = rc.postSendSingle(
                ReliableConnection::RdmaWrite, quorum::pack(quorum::EntryWr, pid, 1),
                address, static_cast<uint32_t>(size), rc.remoteBuf() + dest[pid]);
            if (!ok) return false;
        }
        return true;
    }

    bool BE_Broadcast::broadcastCallWithDep(uint8_t* payload, size_t len, int* deps, bool override = false)
    {
        return broadcastCall(payload, len, deps, override);
    }

    bool BE_Broadcast::broadcastCallNoDep(uint8_t* payload, size_t len, bool override = false)
    {
        return broadcastCall(payload, len, NULL, override);
    }
}