#pragma once

#include <vector>

//#include "branching.hpp"
//#include "error.hpp"

#include <dory/shared/error.hpp>
#include <dory/shared/branching.hpp>
#include "quorum-waiter.hpp"
#include "timers.h"
#include "leader.hpp"

namespace dory {
template <class QuorumWaiter, class ErrorType>
class FixedSizeMajorityOperation {
 public:
  FixedSizeMajorityOperation() {}
  FixedSizeMajorityOperation(ConnectionContext *context, QuorumWaiter qw,
                             std::vector<int> &remote_ids)
      : ctx{context}, qw{qw}, kind{qw.kindOfOp()} {
    quorum_size =
        static_cast<int>(quorum::majority(ctx->remote_ids.size() + 1)) - 1;
    replicas_size = static_cast<int>(ctx->remote_ids.size());

    successful_ops.resize(remote_ids.size());
    successful_ops.clear();

    int tolerated_failures =
        static_cast<int>(quorum::minority(ctx->remote_ids.size() + 1));
    failed_majority = FailureTracker(kind, ctx->remote_ids, tolerated_failures);
    failed_majority.track(qw.reqID());

    auto &rcs = ctx->ce.connections();
    for (auto &[pid, rc] : rcs) {
      if (std::find(remote_ids.begin(), remote_ids.end(), pid) !=
          remote_ids.end()) {
        connections.push_back(Conn(pid, &rc));
      }
    }
  }

  FixedSizeMajorityOperation(ConnectionContext *context, QuorumWaiter qw,
                             std::vector<int> &remote_ids,
                             size_t tolerated_failures)
      : ctx{context}, qw{qw}, kind{qw.kindOfOp()} {
    successful_ops.resize(remote_ids.size());
    successful_ops.clear();

    failed_majority = FailureTracker(kind, ctx->remote_ids,
                                     static_cast<int>(tolerated_failures));
    failed_majority.track(qw.reqID());

    auto &rcs = ctx->ce.connections();
    for (auto &[pid, rc] : rcs) {
      if (std::find(remote_ids.begin(), remote_ids.end(), pid) !=
          remote_ids.end()) {
        connections.push_back(Conn(pid, &rc));
      }
    }
  }

  typename QuorumWaiter::ReqIDType reqID() { return qw.reqID(); }

  void recoverFromError(std::unique_ptr<MaybeError> &supplied_error) {
    if (supplied_error->type() == ErrorType::value) {
      ErrorType &error = *dynamic_cast<ErrorType *>(supplied_error.get());

      auto req_id = error.req();

      // // Just for testing:
      // failed_majority = FailureTracker(kind, ctx->remote_ids, 3);
      // quorum_size = 1;
      // qw.changeQuorum(quorum_size);

      failed_majority.reset();
      failed_majority.track(req_id);
      qw.reset(req_id);

      // TODO (question):
      // To reuse the same req_id, doe we need to make sure no outstanding
      // requests are on the wire?
    } else {
      throw std::runtime_error("Unimplemented handing of this error");
    }
  }

  std::unique_ptr<MaybeError> read(std::vector<void *> &to_local_memories,
                                   size_t size,
                                   std::vector<uintptr_t> &from_remote_memories,
                                   std::atomic<Leader> &leader) {
    std::vector<size_t> size_v(from_remote_memories.size());
    std::fill(size_v.begin(), size_v.end(), size);
    return op_with_leader_bail(ReliableConnection::RdmaRead, to_local_memories,
                               size_v, from_remote_memories, leader);
  }

  std::unique_ptr<MaybeError> read(std::vector<void *> &to_local_memories,
                                   std::vector<size_t> &size,
                                   std::vector<uintptr_t> &from_remote_memories,
                                   std::atomic<Leader> &leader) {
    return op_with_leader_bail(ReliableConnection::RdmaRead, to_local_memories,
                               size, from_remote_memories, leader);
  }

  std::unique_ptr<MaybeError> write(void *from_local_memory, size_t size,
                                    std::vector<uintptr_t> &to_remote_memories,
                                    std::atomic<Leader> &leader) {
    std::vector<size_t> size_v(to_remote_memories.size());
    std::fill(size_v.begin(), size_v.end(), size);
    return op_with_leader_bail(ReliableConnection::RdmaWrite, from_local_memory,
                               size_v, to_remote_memories, leader);
  }

  std::unique_ptr<MaybeError> write(void *from_local_memory,
                                    std::vector<size_t> &size,
                                    std::vector<uintptr_t> &to_remote_memories,
                                    std::atomic<Leader> &leader) {
    return op_with_leader_bail(ReliableConnection::RdmaWrite, from_local_memory,
                               size, to_remote_memories, leader);
  }

  std::unique_ptr<MaybeError> write(std::vector<void *> &from_local_memories,
                                    std::vector<size_t> &size,
                                    std::vector<uintptr_t> &to_remote_memories,
                                    std::atomic<Leader> &leader) {
    return op_with_leader_bail(ReliableConnection::RdmaWrite,
                               from_local_memories, size, to_remote_memories,
                               leader);
  }

  template <typename Poller>
  std::unique_ptr<MaybeError> read(std::vector<void *> &to_local_memories,
                                   size_t size,
                                   std::vector<uintptr_t> &from_remote_memories,
                                   Poller p) {
    return op_without_leader(to_local_memories, size, from_remote_memories, p);
  }

  std::unique_ptr<MaybeError> read(
      std::vector<void *> &to_local_memories, size_t size,
      std::vector<uintptr_t> &from_remote_memories) {
    return op_without_leader(to_local_memories, size, from_remote_memories,
                             ctx->cb.pollCqIsOK);
  }

  template <typename Poller>
  std::unique_ptr<MaybeError> write(void *from_local_memory, size_t size,
                                    std::vector<uintptr_t> &to_remote_memories,
                                    Poller p) {
    return op_without_leader(from_local_memory, size, to_remote_memories, p);
  }

  std::unique_ptr<MaybeError> write(
      void *from_local_memory, size_t size,
      std::vector<uintptr_t> &to_remote_memories) {
    return op_without_leader(from_local_memory, size, to_remote_memories,
                             ctx->cb.pollCqIsOK);
  }

  bool fastWrite(void *from_local_memory, size_t size,
                 std::vector<uintptr_t> &to_remote_memories, uintptr_t offset,
                 std::atomic<Leader> &leader, int outstanding_req) {
    IGNORE(leader);

    auto req_id = qw.fetchAndIncFastID();
    auto next_req_id = qw.nextFastReqID();

    for (auto &c : connections) {
      auto ok = c.rc->postSendSingleCached(
          ReliableConnection::RdmaWrite,
          QuorumWaiter::packer(kind, c.pid, req_id), from_local_memory,
          static_cast<uint32_t>(size),
          c.rc->remoteBuf() + to_remote_memories[c.pid] + offset);

      if (!ok) {
        return false;
      }
    }

    int expected_nr = outstanding_req * replicas_size + quorum_size;
    auto cq = ctx->cq.get();
    entries.resize(expected_nr);
    int num = 0;
    int loops = 0;
    constexpr unsigned mask = (1 << 14) - 1;  // Must be power of 2 minus 1

    while (!qw.canContinueWithOutstanding(outstanding_req, next_req_id)) {
      num = ibv_poll_cq(cq, expected_nr, &entries[0]);
      if (num >= 0) {
        if (!qw.fastConsume(entries, num, expected_nr)) {
          return false;
        }
      } else {
        return false;
      }
    }

    loops = (loops + 1) & mask;
    if (loops == 0) {
      auto ldr = leader.load();
      if (ldr.requester != ctx->my_id) {
        return false;
      }
    }

    range_start = req_id;
    range_end = qw.reqID();

    return true;
  }

  std::unique_ptr<MaybeError> fastWriteError() {
    auto req_id = qw.reqID();
    return std::make_unique<ErrorType>(req_id);
  }

  std::vector<int> &successes() { return successful_ops; }

  uint64_t latestReplicatedID() { return uint64_t(qw.reqID()); }

 private:
  template <class T>
  std::unique_ptr<MaybeError> op_with_leader_bail(
      ReliableConnection::RdmaReq rdma_req, T const &local_memory,
      std::vector<size_t> &size, std::vector<uintptr_t> &remote_memory,
      std::atomic<Leader> &leader) {
    successful_ops.clear();

    auto req_id = qw.reqID();
    auto next_req_id = qw.nextReqID();

    for (auto &c : connections) {
      auto pid = c.pid;
      auto &rc = *(c.rc);
      if constexpr (std::is_same_v<T, std::vector<void *>>) {
        auto ok = rc.postSendSingle(
            rdma_req, QuorumWaiter::packer(kind, pid, req_id),
            local_memory[pid], static_cast<uint32_t>(size[pid]),
            rc.remoteBuf() + remote_memory[pid]);
        if (!ok) {
          return std::make_unique<ErrorType>(req_id);
        }
      } else {
        auto ok =
            rc.postSendSingle(rdma_req, QuorumWaiter::packer(kind, pid, req_id),
                              local_memory, static_cast<uint32_t>(size[pid]),
                              rc.remoteBuf() + remote_memory[pid]);
        if (!ok) {
          return std::make_unique<ErrorType>(req_id);
        }
      }
    }

    int expected_nr = static_cast<int>(connections.size());
    int loops = 0;
    while (!qw.canContinueWith(next_req_id)) {
      entries.resize(expected_nr);
      if (ctx->cb.pollCqIsOK(ctx->cq, entries)) {
        if (!qw.consume(entries, successful_ops)) {
          if (failed_majority.isUnrecoverable(entries)) {
            return std::make_unique<ErrorType>(req_id);
          }
        }
      } else {
        std::cout << "Poll returned an error" << std::endl;
        return std::make_unique<ErrorType>(req_id);
      }

      // Workaround: When leader changes, some poll events may get lost
      // (most likely due to a bug on the driver) and we are stuck in an
      // infinite loop.
      loops += 1;
      if (loops % 1024 == 0) {
        loops = 0;
        auto ldr = leader.load();
        if (ldr.requester != ctx->my_id) {
          return std::make_unique<ErrorType>(req_id);
        }
      }
    }

    qw.setFastReqID(next_req_id);
    return std::make_unique<NoError>();
  }

  // if T is a vector, this function sends a read request to all remotes and stores the
  // result in the corresponding index of the "local_memory" input
  // otherwise, it will write the contents of the "local_memory" to all the remote locations
  template <class T, class Poller>
  std::unique_ptr<MaybeError> op_without_leader(
      T const &local_memory, size_t size, std::vector<uintptr_t> &remote_memory,
      Poller poller) {
    successful_ops.clear();

    auto req_id = qw.reqID();
    auto next_req_id = qw.nextReqID();

    for (auto &c : connections) {
      auto pid = c.pid;
      auto &rc = *(c.rc);
      if constexpr (std::is_same_v<T, std::vector<void *>>) {
        auto ok = rc.postSendSingle(
            ReliableConnection::RdmaRead,
            QuorumWaiter::packer(kind, pid, req_id),
            local_memory[pid],
            static_cast<uint32_t>(size), 
            rc.remoteBuf() + remote_memory[pid]);
        if (!ok) {
          return std::make_unique<ErrorType>(req_id);
        }
      } else {
        auto ok = rc.postSendSingle(
            ReliableConnection::RdmaWrite,
            QuorumWaiter::packer(kind, pid, req_id),
            local_memory,
            static_cast<uint32_t>(size),
            rc.remoteBuf() + remote_memory[pid]);
        if (!ok) {
          return std::make_unique<ErrorType>(req_id);
        }
      }
    }

    int expected_nr = static_cast<int>(connections.size());
    while (!qw.canContinueWith(next_req_id)) {
      entries.resize(expected_nr);
      if (poller(ctx->cq, entries)) {
        if (!qw.consume(entries, successful_ops)) {
          if (failed_majority.isUnrecoverable(entries)) {
            return std::make_unique<ErrorType>(req_id);
          }
        }
      } else {
        std::cout << "Poll returned an error" << std::endl;
        return std::make_unique<ErrorType>(req_id);
      }
    }

    qw.setFastReqID(next_req_id);
    return std::make_unique<NoError>();
  }

 private:
  struct Conn {
    Conn(int pid, dory::ReliableConnection *rc) : pid{pid}, rc{rc} {}
    int pid;
    dory::ReliableConnection *rc;
  };

 private:
  ConnectionContext *ctx;

  QuorumWaiter qw;
  quorum::Kind kind;

  int quorum_size, replicas_size;

  FailureTracker failed_majority;

  std::vector<struct ibv_wc> entries;
  std::vector<int> successful_ops;
  std::vector<Conn> connections;

 public:
  uint64_t range_start = 0, range_end = 0;
};
}  // namespace dory