
#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

#include <dory/conn/exchanger.hpp>
#include <dory/conn/rc.hpp>
#include <dory/ctrl/block.hpp>
#include <dory/ctrl/device.hpp>
namespace dory{
    class nonblocking_protocol
    {
    private:
        ControlBlock& cb;
        
    public:
        nonblocking_protocol(ControlBlock& block);
        ~nonblocking_protocol();
    };

    nonblocking_protocol::nonblocking_protocol(dory::ControlBlock& block)
    {
    cb = block;
    cb->registerPD("hamsaz");
    cb->allocateBuffer("deposit-buf", allocated_size, alignment);
    cb->registerMR("deposit-mr", "hamsaz", "deposit-buf",
                    dory::ControlBlock::LOCAL_READ | dory::ControlBlock::LOCAL_WRITE |
                        dory::ControlBlock::REMOTE_READ | dory::ControlBlock::REMOTE_WRITE);
    cb->registerCQ("cq-deposit");
    
    ce_deposit =
        std::make_unique<dory::ConnectionExchanger>(my_id, remote_ids, *cb.get());
    ce_deposit->configure_all("hamsaz", "deposit-mr", "cq-deposit",
                                    "cq-deposit");
    ce_deposit->announce_all(store, "qp-deposit");
    ce_deposit->announce_ready(store, "qp-deposit", "announce");

    auto shared_memory_addr =
        reinterpret_cast<uint8_t*>(cb->mr("deposit-mr").addr);

    overlay =
        std::make_unique<OverlayAllocator>(shared_memory_addr, allocated_size);

    scratchpad =
        std::make_unique<ScratchpadMemory>(ids, *overlay.get(), alignment);
    auto [logmem_ok, logmem, logmem_size] = overlay->allocateRemaining(alignment);

    LOGGER_INFO(logger, "Log allocation for deposit... {}", logmem_ok ? "OK" : "FAILED");
    LOGGER_INFO(logger, "Log deposit(address: 0x{:x}, size: {} bytes)",
                uintptr_t(logmem), logmem_size);

    auto log_offset = logmem - shared_memory_addr;

    replication_log = std::make_unique<Log>(logmem, logmem_size);

    ce_deposit->wait_ready_all(store, "qp-deposit", "announce");
    
    ce_deposit->connect_all(
        store, "qp-deposit",
        dory::ControlBlock::LOCAL_READ | dory::ControlBlock::LOCAL_WRITE);
    ce_deposit->announce_ready(store, "qp-deposit", "connect");

    
    ce_deposit->wait_ready_all(store, "qp-deposit", "connect");
    
    // Initialize the contexts
    auto& cq_deposit = cb->cq("cq-deposit");
    re_conn_ctx = std::make_unique<dory::ConnectionContext>(
        *cb.get(), *ce_deposit.get(), cq_deposit, remote_ids, my_id);
    }

    nonblocking_protocol::~nonblocking_protocol()
    {
    }
}
