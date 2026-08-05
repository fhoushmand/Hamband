#include <stdexcept>
#include <thread>

#include <dory/crash-consensus.hpp>
#include <dory/shared/branching.hpp>

#include "beb.hpp"
#include "synchronizer.hpp"


using namespace band;

class NB_Wellcoordination : Synchronizer {
 public:
  struct LatencyBreakdown {
    uint64_t attempted = 0;
    uint64_t no_error = 0;
    uint64_t not_permissible = 0;
    uint64_t dory_error = 0;
    uint64_t total_ns = 0;
    uint64_t mu_ns = 0;
    uint64_t pre_mu_ns = 0;
    uint64_t post_mu_ns = 0;
  };

  int self;
  size_t num_process;
  std::vector<int> remote_ids;
  std::unique_ptr<ReliableBroadcast> rb;
  std::unique_ptr<dory::Consensus>* tob;
  ReplicatedObject* repl_object;
  bool collect_latency_breakdown = false;
  LatencyBreakdown leader_conflict_breakdown;

  void executeOrBlock(MethodCall call, bool leader);
  bool checkCallDependencies(MethodCall const& callWithDeps);

  ~NB_Wellcoordination() { rb.reset(); }

  NB_Wellcoordination(int id, std::vector<int> r_ids, ReplicatedObject* obj) {
    self = id;
    remote_ids = r_ids;
    num_process = remote_ids.size() + 1;
    repl_object = obj;
    
    rb = std::make_unique<ReliableBroadcast>(id, remote_ids, repl_object);

    tob = new std::unique_ptr<dory::Consensus>[repl_object->synch_groups.size()];
    std::cout << "leaders: " << repl_object->synch_groups.size() << std::endl;
    for (size_t i = 0; i < repl_object->synch_groups.size(); i++)
    {
      if(i == 0)
        tob[i] = std::make_unique<dory::Consensus>(id, remote_ids, 8, dory::ThreadBank::A);
      else
        tob[i] = std::make_unique<dory::Consensus>(id, remote_ids, 8, dory::ThreadBank::B);
      // this is only called in the followers
      // in the leader, after proposal we directly call response
      // since we know that it can be delivered to the leader right away
      // ie., call to the handler and return of the propose method are equivalent
      tob[i]->commitHandler([&]([[maybe_unused]] bool leader,
                                        [[maybe_unused]] uint8_t* buf,
                                        [[maybe_unused]] size_t len) {
        // std::cout << "commit" << std::endl;
        MethodCall request = repl_object->deserialize(buf);
        // check execution and block if dependencies are not satisfied
        executeOrBlock(request, leader);
      });
    }
  }

  virtual std::pair<ResponseStatus,std::chrono::high_resolution_clock::time_point> request(MethodCall request, bool debug, bool summarize) {
    std::chrono::high_resolution_clock::time_point breakdown_start;
    if (collect_latency_breakdown) {
      breakdown_start = std::chrono::high_resolution_clock::now();
    }
    // a query method
    // handle localy and do not propagate
    // std::cout << "request: " << request.id << std::endl;
    // std::cout << "read_methods: " << repl_object->read_methods.size() << std::endl;
    if(std::find(repl_object->read_methods.begin(), repl_object->read_methods.end(), request.method_type) != repl_object->read_methods.end())
    {
      repl_object->execute(request);
      return response(request, ResponseStatus::NoError, false);
    }

    std::vector<uint8_t> payload_buffer(repl_object->serializedSize(request));
    uint8_t* payload = &payload_buffer[0];

    // std::cout << "serialize" << request.method_type << std::endl;
    auto length = repl_object->serialize(request, payload);
    payload_buffer.resize(length);
    int synch_gp = repl_object->getSynchGroup(request.method_type);
    bool collect_conflict_breakdown = collect_latency_breakdown && synch_gp != -1;
    
    if (!repl_object->isPermissible(request)) {
        std::cout << "not permissible, dropping request " << request.method_type << std::endl;
        auto ret = response(request, ResponseStatus::NotPermissible, debug);
        if (collect_conflict_breakdown) {
          leader_conflict_breakdown.attempted++;
          leader_conflict_breakdown.not_permissible++;
          leader_conflict_breakdown.total_ns +=
              std::chrono::duration_cast<std::chrono::nanoseconds>(
                  ret.second - breakdown_start)
                  .count();
        }
        return ret;
    }
    // non-conflicting calls
    // no need to defer the permissibility check
    // can execute in-place (however, check for the permissibility right away before execution)
    if (synch_gp == -1) {
      // execute locally
      repl_object->internalExecute(request, self - 1);
      
      //reliable broadcast
      rb->broadcast(payload, length, summarize);

      return response(request, ResponseStatus::NoError, debug);
    }
    // conflicting call
    else {
      if (collect_conflict_breakdown) {
        leader_conflict_breakdown.attempted++;
      }
      std::chrono::high_resolution_clock::time_point before_mu;
      if (collect_conflict_breakdown) {
        before_mu = std::chrono::high_resolution_clock::now();
      }
      dory::ProposeError err;
      do {
        err = tob[synch_gp]->propose(payload, payload_buffer.size());
        if (err == dory::ProposeError::SlowPathLogRecycled) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        } else if (err != dory::ProposeError::NoError) {
          std::this_thread::yield();
        }
      } while (err != dory::ProposeError::NoError);
      std::chrono::high_resolution_clock::time_point after_mu;
      if (collect_conflict_breakdown) {
        after_mu = std::chrono::high_resolution_clock::now();
      }
      // std::cout << "after propose" << std::endl;
      // executed with no error - sending the response.
      auto ret = response(request, ResponseStatus::NoError, debug);
      if (collect_conflict_breakdown) {
        leader_conflict_breakdown.no_error++;
        leader_conflict_breakdown.mu_ns +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                after_mu - before_mu)
                .count();
        leader_conflict_breakdown.pre_mu_ns +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                before_mu - breakdown_start)
                .count();
        leader_conflict_breakdown.post_mu_ns +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                ret.second - after_mu)
                .count();
        leader_conflict_breakdown.total_ns +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                ret.second - breakdown_start)
                .count();
      }
      return ret;
    }
  }
};

void NB_Wellcoordination::executeOrBlock(MethodCall call, bool leader) {
  while (!leader && !checkCallDependencies(call)) {
    std::this_thread::yield();
  }
  size_t separator = call.id.find('-');
  if (separator == std::string::npos) {
    throw std::runtime_error("Method-call id does not contain an origin");
  }
  int origin = std::stoi(call.id.substr(0, separator)) - 1;
  if (origin < 0 || static_cast<size_t>(origin) >= num_process) {
    throw std::runtime_error("Method-call origin is outside the replica set");
  }
  repl_object->internalExecute(call, static_cast<size_t>(origin));
  return;
}

bool NB_Wellcoordination::checkCallDependencies(MethodCall const& callWithDeps) {
  if (repl_object->dependency_relation.find(callWithDeps.method_type) == repl_object->dependency_relation.end()) return true;
  for (size_t x = 0; x < repl_object->dependency_relation[callWithDeps.method_type].size(); x++) {
    int dependency_method = repl_object->dependency_relation[callWithDeps.method_type][x];
    for (size_t i = 0; i < num_process; i++){
      if (repl_object->calls_applied[dependency_method][i] <
          callWithDeps.dependency_vectors[x][i])
        return false;
    }
  }
  return true;
}
