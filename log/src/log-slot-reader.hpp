#pragma once
#include <dory/mem/memory.hpp>

#include "remote-log-reader.hpp"
#include "context.hpp"

namespace dory {
struct Leader;
class LogSlotReader {
 public:
  // TODO: Eliminate duplication in the two constructors
  LogSlotReader(ReplicationContext *context, ScratchpadMemory &scratchpad,
                uintptr_t iterators_offset)
      : r_ctx{context}, c_ctx{&context->cc}, scratchpad{scratchpad} {
    remote_iterators.resize(
        Identifiers::maxID(c_ctx->my_id, c_ctx->remote_ids) + 1);

    entry_read_req_id = 1;
    to_be_polled = ToBePolled(quorum::EntryRd, c_ctx->remote_ids);

    for (auto id : c_ctx->remote_ids) {
      auto iter = r_ctx->log.remoteIterator(id, iterators_offset);
      remote_iterators[id] = WrappedRemoteIterator(iter);
    }

    // Exclude myself from the quorum
    quorum_size = quorum::majority(c_ctx->remote_ids.size() + 1) - 1;

    successful_reads.resize(c_ctx->remote_ids.size());
    successful_reads.clear();

    tolerated_failures = quorum::minority(c_ctx->remote_ids.size() + 1);
  }

  // This function is used only to test the recovery
  void addQuorumSize(int term) { quorum_size += term; }

  // This function is used only to test the recovery
  void addToleratedFailures(int term) { tolerated_failures += term; }

  void recoverFromError(std::unique_ptr<MaybeError> &supplied_error) {
    if (supplied_error->type() == MaybeError::ReadLogMajorityError) {
      ReadLogMajorityError &error =
          *dynamic_cast<ReadLogMajorityError *>(supplied_error.get());

      auto req_id = error.req();

      entry_read_req_id = req_id;

      /*
      // To reuse the same entry_read_req_id, we need to make sure no
      // outstanding requests are on the wire
      unsigned expected_nr = to_be_polled.pollList().size();
      unsigned responses = 0;
      while (responses < expected_nr) {
        entries.resize(expected_nr);
        if (ctx->cb.pollCqIsOK(ctx->cq, entries)) {
          auto [positive_resp, negative_resp] =
              to_be_polled.actuallyPolled(entries);
          responses += positive_resp.get().size() + negative_resp.get().size();
        } else {
          std::cout << "Poll returned an error" << std::endl;
        }
      }
      */
      // Reset polling tracking
      to_be_polled = ToBePolled(quorum::EntryRd, c_ctx->remote_ids);

      failed_pids.clear();
    }
  }

  std::unique_ptr<MaybeError> readSlotAt(uint64_t remote_offset, std::atomic<Leader> &leader) {
    to_be_polled.focusOnReqID(entry_read_req_id);
    to_be_polled.rescheduleCompleted();

    successful_reads.clear();
    auto &rcs = c_ctx->ce.connections();

    auto wait_for = quorum_size;

    int loops = 0;
    do {
      ptrdiff_t offset;
      uint32_t size;
      std::cout << "inside while\n" << std::endl;
      if (!to_be_polled.postList().empty()) {
        for (auto &pid : to_be_polled.postList()) {
          auto &rc = rcs.find(pid)->second;
          auto store_addr = scratchpad.readLogEntrySlots()[pid];

          auto offset_size =
              remote_iterators[pid].iterator().lookAt(remote_offset);
          offset = offset_size.first;
          size = static_cast<uint32_t>(offset_size.second);
          remote_iterators[pid].iterator().storeDest(store_addr);

          auto ok = rc.postSendSingle(
              ReliableConnection::RdmaRead,
              quorum::pack(quorum::EntryRd, pid, entry_read_req_id), store_addr,
              size, rc.remoteBuf() + r_ctx->log_offset + offset);

          if (!ok) {
            return std::make_unique<ReadLogMajorityError>(entry_read_req_id);
          }
        }

        to_be_polled.posted();
      }

      auto expected_nr = to_be_polled.pollList().size();
      size_t responses = 0;

      // // This loop in not necessary. We believe it improves performance
      // while (responses < std::min(wait_for, expected_nr)) {
      entries.resize(expected_nr);
      if (c_ctx->cb.pollCqIsOK(c_ctx->cq, entries)) {
        auto [positive_resp, negative_resp] =
            to_be_polled.actuallyPolled(entries);

        // On the positive responses, try to move the iterator.
        for (auto pid : positive_resp.get()) {
          auto &it = remote_iterators[pid];
          if (it.isPopulated()) {
            if (it.isComplete()) {
              to_be_polled.moveToCompleted(pid);
              successful_reads.push_back(pid);
            }
          } else {
            to_be_polled.moveToCompleted(pid);
            successful_reads.push_back(-pid);
          }
        }

        // The negative responses have already been excluded from
        // `ToBePolled`.
        if (!negative_resp.get().empty()) {
          for (auto pid : negative_resp.get()) {
            failed_pids.insert(pid);
          }

          if (failed_pids.size() > tolerated_failures) {
            return std::make_unique<ReadLogMajorityError>(entry_read_req_id);
          }
        }

        responses += entries.size();
      } else {
        std::cout << "Poll returned an error" << std::endl;
        return std::make_unique<ReadLogMajorityError>(entry_read_req_id);
      }

      // Workaround: When leader changes, the some poll events may get lost
      // (most likely due to a bug on the driver) and we are stuck in an
      // infinite loop.
      // loops += 1;
      // if (loops % 1024 == 0) {
      //   loops = 0;
      //   auto ldr = leader.load();
      //   if (ldr.requester != c_ctx->my_id) {
      //     return std::make_unique<ReadLogMajorityError>(entry_read_req_id);
      //   }
      // }
      // }

    } while (to_be_polled.completedList().size() < wait_for);

    entry_read_req_id += 1;
    return std::make_unique<NoError>();
  }

  std::vector<int> &successes() { return successful_reads; }

 private:
  ReplicationContext *r_ctx;
  ConnectionContext *c_ctx;
  ScratchpadMemory &scratchpad;

  std::vector<WrappedRemoteIterator> remote_iterators;
  ToBePolled to_be_polled;

  size_t quorum_size;
  size_t tolerated_failures;
  uint64_t entry_read_req_id;

  std::vector<int> successful_reads;
  std::vector<struct ibv_wc> entries;

  std::set<int> failed_pids;
};
}