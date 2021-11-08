#include <thread>

#include <dory/crash-consensus.hpp>
#include <dory/shared/branching.hpp>

#include "beb.hpp"
#include "courseware.hpp"
#include "readerwriterqueue.h"
#include "synchronizer.hpp"
#include <mutex>

using namespace hamsaz;

class NB_Wellcoordination : Synchronizer {
  //  private:
 public:
  // common in all the use-cases
  int self;
  size_t num_process;
  std::vector<int> remote_ids;
  uint64_t req_id = 0;
  std::vector<ptrdiff_t> partitions_offset;
  std::unique_ptr<BE_Broadcast> eventualSender;
  std::unique_ptr<dory::Consensus> synchSenders;
  std::vector<MethodCall>* pending_dependencies;
  std::set<size_t>* executed_pending_index;
  std::unordered_map<std::string, uint64_t>* response_times;
  std::atomic<int>** calls_applied;
  uint64_t throughput_end;
  MethodCallFactory callFactory;

  
  std::recursive_mutex cs_lock;
  std::recursive_mutex es_cs_lock;
  Courseware courseware = Courseware();

  // int pre_state;
  // int withdrawn_amount = 0;
  // std::atomic<int> local_state = 0;
  // std::atomic<int> receive_states = 0;

  const uint64_t QUEUE_SIZE = 10000;

  ~NB_Wellcoordination();
  void readLogParts(BE_Broadcast& sender, size_t i);
  int checkPendingDependencies();
  bool executeOrAddPendingDependencies(MethodCall call);
  bool checkCallDependencies(MethodCall callWithDeps);
  void execute(MethodCall call, size_t origin);

  NB_Wellcoordination(int id, std::vector<int> r_ids) {
    self = id;
    remote_ids = r_ids;
    num_process = remote_ids.size() + 1;
    partitions_offset.resize(num_process);
    callFactory = MethodCallFactory(courseware, num_process);
    calls_applied = new std::atomic<int>*[courseware.num_methods];
    for (int i = 0; i < courseware.num_methods; i++)
      calls_applied[i] = new std::atomic<int>[num_process];
    for (int x = 0; x < courseware.num_methods; x++)
      for (size_t i = 0; i < num_process; i++) calls_applied[x][i] = 0;

    pending_dependencies = new std::vector<MethodCall>[callFactory.total_dependent_methods];
    executed_pending_index = new std::set<size_t>[callFactory.total_dependent_methods];
    // for (size_t i = 0; i < callFactory.total_dependent_methods; i++)
    // {
    //   pending_dependencies[i] = std::vector<MethodCall>(QUEUE_SIZE);
    //   executed_pending_index[i] = std::set<size_t>(QUEUE_SIZE);
    // }

    response_times = new std::unordered_map<std::string, uint64_t>[courseware.num_methods];
    for(int i = 0; i < courseware.num_methods; i++)
      response_times[i] = std::unordered_map<std::string,uint64_t>();

    // each partition has an offset of:
    // (LogConfig::LOG_SIZE/num_process)*part_num
    for (size_t i = 0; i < num_process; i++) {
      partitions_offset[i] =
          LogConfig::round_up_powerof2(LogConfig::LOG_SIZE / num_process) * i;
    }

    eventualSender = std::make_unique<BE_Broadcast>(id, remote_ids);
    // for (size_t i = 0; i < courseware.synch_groups.size(); i++)
      synchSenders = std::make_unique<dory::Consensus>(id, remote_ids, 16,
                                                           dory::ThreadBank::A);

    // this is only called in the followers
    // in the leader, after proposal we directly call response
    // since we know that it can be delivered to the leader right away
    // ie., call to the handler and return of the propose method are equivalent
    synchSenders->commitHandler([&]([[maybe_unused]] bool leader,
                                      [[maybe_unused]] uint8_t* buf,
                                      [[maybe_unused]] size_t len) {
      MethodCall response = callFactory.deserialize(buf);
      std::cout << "received call:" << std::endl;
      // callFactory.toString(response);
      // execute conflicting call (add dependency check here)
      if (!executeOrAddPendingDependencies(response)) {
        std::cout << response.id << " queued for later exec" << std::endl;
      }
      checkPendingDependencies();
      // withdrawn_amount += response.arg;
    });

    std::thread nonConflictingLogReader([&] {
      while (true) {
        for (auto& i : remote_ids) readLogParts(*eventualSender.get(), i - 1);
        // account.balance = pre_state + receive_states + local_state -
        // withdrawn_amount; need to wait here otherwise writes are not done
        // from remote and the results are not correct
        std::this_thread::sleep_for(std::chrono::microseconds(200));
      }
      return;
    });
    nonConflictingLogReader.detach();

    // this causes data-race because withdrawn_amount is updated in multiple
    // threads std::thread depsCheck([&] {
    //   while (true) {
    //     checkPendingDependencies();
    //     std::this_thread::sleep_for(std::chrono::microseconds(50));
    //   }
    // });
    // depsCheck.detach();
  }

  virtual void request(std::string input, bool debug, bool summarize) {
    std::vector<uint8_t> payload_buffer(256);
    uint8_t* payload = &payload_buffer[0];
    std::string sequence_number = std::to_string(self) + "-" + std::to_string(req_id++);
    

    MethodCall request = callFactory.createCall(sequence_number, input);

    // if(debug)
    // {
    //   for (int x = 0; x < courseware.num_methods; x++)
    //   {
    //     std::cout << "method " << x + 1 << ": " << std::endl;
    //     for (size_t i = 0; i < num_process; i++)
    //       std::cout << calls_applied[x][i].load() << ", ";
    //     std::cout << std::endl;
    //   }
    // }

    uint64_t start = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch())
                .count();
    response_times[request.method_type][sequence_number] = start;
    // a query method
    // handle localy and do not propagate
    if (std::find(courseware.update_methods.begin(), courseware.update_methods.end(), request.method_type) == courseware.update_methods.end()) {
      // account.balance += receive_states;
      courseware.toString();
      response(request, 1, debug);
      return;
    }

    auto dependency_size =
        courseware.dependency_relation[request.method_type].size();
    if (dependency_size != 0) {
      int** vector = new int*[dependency_size];
      for (size_t i = 0; i < dependency_size; i++) {
        int* vs = new int[num_process];
        for (size_t j = 0; j < num_process; j++)
          vs[j] = calls_applied[courseware.dependency_relation[request.method_type][i]][j].load();
        vector[i] = vs;
      }
      request.attachDependencies(vector);
    }
    auto length = callFactory.serialize(request, payload);
    payload_buffer.resize(length);

    

    int synch_gp = courseware.getSynchGroup(request.method_type);
    
    // non-conflicting calls
    // no need to defer the permissibility check
    // can execute in-place (however, check for the permissibility right away
    // before execution)
    if (synch_gp == -1) {
      // deposit_response_time[idStr] = start;
      // invariant sufficient call
      // always permissibile
      // execute locally
      // read your own writes style
      if (!courseware.isPermissible(request)) {
        std::cout << "not permissible, dropping request" << std::endl;
        return;
      }
      courseware.execute(request);
      calls_applied[request.method_type][self - 1]++;

      Call call(*(eventualSender->log.get()), partitions_offset[self - 1],
                payload, length, summarize);
      auto [address, offset, size] = call.location();

      for (auto& i : remote_ids) {
        auto pid = i - 1;

        // std::cout << "writing " << payload << " to: " << i << " at: " <<
        // std::hex
        //           << eventualSender->ce->connections().at(i).remoteBuf() +
        //           offset
        //           << std::endl;
        auto ok = eventualSender->ce->connections().at(i).postSendSingleCached(
            ReliableConnection::RdmaWrite,
            quorum::pack(quorum::EntryWr, i, req_id), address,
            static_cast<uint32_t>(size),
            eventualSender->ce->connections().at(i).remoteBuf() + offset);
      }
      eventualSender->pollCQ();

      response(request, 1, debug);
    } else if (synch_gp != -1) {
      if (unlikely(debug)) {
        std::cout << "executed pendings size : "
                  << executed_pending_index[request.method_type].size() << std::endl;
        std::cout << "pendings size : " << pending_dependencies[request.method_type].size()
                  << std::endl;
      }
      if (!courseware.isPermissible(request)) {
        std::cout << "not permissible, dropping request" << std::endl;
        return;
      }

      dory::ProposeError err =
          synchSenders->propose(payload, payload_buffer.size());
      response(request, 1, false);
      if (err != dory::ProposeError::NoError) {
        switch (err) {
          case dory::ProposeError::FastPath:
          case dory::ProposeError::FastPathRecyclingTriggered:
          case dory::ProposeError::SlowPathCatchFUO:
          case dory::ProposeError::SlowPathUpdateFollowers:
          case dory::ProposeError::SlowPathCatchProposal:
          case dory::ProposeError::SlowPathUpdateProposal:
          case dory::ProposeError::SlowPathReadRemoteLogs:
          case dory::ProposeError::SlowPathWriteAdoptedValue:
          case dory::ProposeError::SlowPathWriteNewValue:
            std::cout << "Error: in leader mode. Code: "
                      << static_cast<int>(err) << std::endl;
            break;

          case dory::ProposeError::SlowPathLogRecycled:
            std::cout << "Log recycled, waiting a bit..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            break;

          case dory::ProposeError::MutexUnavailable:
          case dory::ProposeError::FollowerMode:
            std::cout << "Error: in follower mode. Potential leader: "
                      << synchSenders->potentialLeader() << std::endl;
            break;

          default:
            std::cout << "Bug in code. You should only handle errors here"
                      << std::endl;
        }
      }

    } else {
      std::cout << "invalid request" << std::endl;
      return;
    }
    return;
  }

  virtual void response(MethodCall request, int response, bool debug) {
    auto end = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch())
                   .count();

    response_times[request.method_type][request.id] =
        (end - response_times[request.method_type][request.id]);
    if (debug)
      std::cout << "response for " << request.id << ": " << response
                << std::endl;
  }
};

NB_Wellcoordination::~NB_Wellcoordination() { eventualSender.reset(); }

void NB_Wellcoordination::readLogParts(BE_Broadcast& sender, size_t i) {
  while (sender.partIter.hasNext(i)) {
    ParsedCall pslot = ParsedCall(sender.partIter.location(i));
    auto [buf, len] = pslot.payload();
    if (!pslot.isPopulated()) break;
    MethodCall call = callFactory.deserialize(buf);
    courseware.execute(call);
    calls_applied[call.method_type][i]++;
    sender.partIter.next(i);
  }
  return;
}

bool NB_Wellcoordination::executeOrAddPendingDependencies(MethodCall call) {
  if (checkCallDependencies(call)) {
    courseware.execute(call); 
    calls_applied[call.method_type][self - 1]++;
    return true;
  } else {
    pending_dependencies[call.method_type].push_back(call);
    return false;
  }
}

bool NB_Wellcoordination::checkCallDependencies(MethodCall callWithDeps) {
  if (courseware.dependency_relation.find(callWithDeps.method_type) == courseware.dependency_relation.end()) return true;
  std::cout << "has " << courseware.dependency_relation[callWithDeps.method_type].size() << " deps to check" << std::endl;
  for (size_t x = 0; x < courseware.dependency_relation[callWithDeps.method_type].size(); x++) {
    int dependency_method = courseware.dependency_relation[callWithDeps.method_type][x];
    std::cout << "dep method: " << dependency_method << std::endl;
    for (size_t i = 0; i < num_process; i++){
      std::cout << "applied: " << calls_applied[dependency_method][i] << std::endl;
      std::cout << "call: " << callWithDeps.dependency_vectors[x][i] << std::endl;
      if (calls_applied[dependency_method][i] <
          callWithDeps.dependency_vectors[x][i])
        return false;
    }
  }
  std::cout << "deps check finished" << std::endl;
  

  return true;
}

int NB_Wellcoordination::checkPendingDependencies() {
  for (size_t x = 0; x < callFactory.total_dependent_methods; x++) {
    for (size_t i = 0; i < pending_dependencies[x].size(); i++) {
      if (executed_pending_index[x].find(i) ==
          executed_pending_index[x].end()) {
        MethodCall call = pending_dependencies[x][i];
        // std::cout << "checking " << pendingG.id << " with ";
        // for (size_t i = 0; i < num_process; i++)
        // std::cout << pending.deps[i] << ", ";
        // std::cout << std::endl;
        if (checkCallDependencies(call)) {
          std::cout << call.id << " found in queue, executing..." << std::endl;
          courseware.execute(call);
          executed_pending_index[x].insert(i);
          // executing a pending call
          calls_applied[x]++;
        }
      }
    }
  }
  return 0;
}
