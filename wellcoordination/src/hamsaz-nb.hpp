#include <thread>

#include <dory/crash-consensus.hpp>
#include <dory/shared/branching.hpp>

#include "beb.hpp"
#include "synchronizer.hpp"


using namespace hamsaz;

class NB_Wellcoordination : Synchronizer {
 public:
  // common in all the use-cases
  int self;
  uint64_t next_id;
  uint64_t fast_id;
  size_t num_process;
  std::vector<int> remote_ids;
  std::vector<ptrdiff_t> partitions_offset;
  std::unique_ptr<BE_Broadcast> eventualSender;
  std::unique_ptr<dory::Consensus>* synchSenders;
  std::atomic<int>** calls_applied;
  uint64_t throughput_end;
  MethodCallFactory callFactory;
  ReplicatedObject* repl_object;

  ~NB_Wellcoordination() { eventualSender.reset(); }

  void readLogParts(BE_Broadcast& sender, size_t i);
  void executeOrBlock(MethodCall call, bool leader, int l);
  bool checkCallDependencies(MethodCall callWithDeps);

  NB_Wellcoordination(int id, std::vector<int> r_ids, ReplicatedObject* obj) {
    self = id;
    remote_ids = r_ids;
    num_process = remote_ids.size() + 1;
    repl_object = obj;
    partitions_offset.resize(num_process);
    callFactory = MethodCallFactory(repl_object, num_process);
    calls_applied = new std::atomic<int>*[repl_object->num_methods];
    for (int i = 0; i < repl_object->num_methods; i++)
      calls_applied[i] = new std::atomic<int>[num_process];
    for (int x = 0; x < repl_object->num_methods; x++)
      for (size_t i = 0; i < num_process; i++) calls_applied[x][i] = 0;

    // each partition has an offset of:
    // (LogConfig::LOG_SIZE/num_process)*part_num
    for (size_t i = 0; i < num_process; i++) {
      partitions_offset[i] =
          LogConfig::round_up_powerof2(LogConfig::LOG_SIZE / num_process) * i + 1024;
    }

    eventualSender = std::make_unique<BE_Broadcast>(id, remote_ids);

    synchSenders = new std::unique_ptr<dory::Consensus>[repl_object->synch_groups.size()];
    std::cout << "leaders: " << repl_object->synch_groups.size() << std::endl;
    for (size_t i = 0; i < repl_object->synch_groups.size(); i++)
    {
      if(i == 0)
        synchSenders[i] = std::make_unique<dory::Consensus>(id, remote_ids, 8, dory::ThreadBank::A);
      else
        synchSenders[i] = std::make_unique<dory::Consensus>(id, remote_ids, 8, dory::ThreadBank::B);
      // this is only called in the followers
      // in the leader, after proposal we directly call response
      // since we know that it can be delivered to the leader right away
      // ie., call to the handler and return of the propose method are equivalent
      synchSenders[i]->commitHandler([&]([[maybe_unused]] bool leader,
                                        [[maybe_unused]] uint8_t* buf,
                                        [[maybe_unused]] size_t len) {
        MethodCall* request = callFactory.deserialize(buf);
        // check execution and block if dependencies are not satisfied
        executeOrBlock(*request, leader, static_cast<int>(i));
      });
    }

    std::thread nonConflictingLogReader([&] {
      while (true) {
        for (auto& i : remote_ids) 
          readLogParts(*eventualSender.get(), i - 1);
        std::this_thread::sleep_for(std::chrono::microseconds(400));
      }
      return;
    });
    nonConflictingLogReader.detach();
  }


  inline uint64_t fetchAndIncFastID() {
    auto ret = fast_id;
    fast_id += 1;
    return ret;
  }

  inline uint64_t nextFastReqID() const { return fast_id; }

  virtual std::pair<ResponseStatus,std::chrono::high_resolution_clock::time_point> request(MethodCall request, bool debug, bool summarize) {
    // a query method
    // handle localy and do not propagate
    if (request.method_type == repl_object->read_method) 
      return response(request, ResponseStatus::NoError, false);

    
    std::vector<uint8_t> payload_buffer(256);
    uint8_t* payload = &payload_buffer[0];
    auto dependency_size =
        repl_object->dependency_relation[request.method_type].size();
    if (dependency_size != 0) {
      int** vector = new int*[dependency_size];
      for (size_t i = 0; i < dependency_size; i++) {
        int* vs = new int[num_process];
        for (size_t j = 0; j < num_process; j++)
          vs[j] = calls_applied[repl_object->dependency_relation[request.method_type][i]][j];
        vector[i] = vs;
      }
      request.attachDependencies(vector);
    }
    auto length = callFactory.serialize(request, payload);
    payload_buffer.resize(length);
    int synch_gp = repl_object->getSynchGroup(request.method_type);
    
    if (!repl_object->isPermissible(request)) {
        std::cout << "not permissible, dropping request " << request.method_type << std::endl;
        return response(request, ResponseStatus::NotPermissible, debug);
    }
    // non-conflicting calls
    // no need to defer the permissibility check
    // can execute in-place (however, check for the permissibility right away
    // before execution)
    if (synch_gp == -1) {
      // execute locally
      // read your own writes style
      repl_object->execute(request);
      calls_applied[request.method_type][self - 1]++;

      Call call(*(eventualSender->log.get()), partitions_offset[self - 1],
                payload, length, summarize);
      auto [address, offset, size] = call.location();
      
      auto req_id = fetchAndIncFastID();
      auto next_req_id = nextFastReqID();

      // if(unlikely(req_id % 999 == 0))
      if(true)
      {
        for (auto& i : remote_ids) {
          auto ok = eventualSender->ce->connections().at(i).postSendSingle(
              ReliableConnection::RdmaWrite,
              quorum::pack(quorum::EntryWr, i, req_id), address,
              static_cast<uint32_t>(size),
              eventualSender->ce->connections().at(i).remoteBuf() + offset);
        }
        eventualSender->pollCQ();
        // std::cout << "time spent: " <<  std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() << std::endl;
      }
      else
      {
        for (auto& i : remote_ids) {
          auto ok = eventualSender->ce->connections().at(i).postSendSingleNoSignal(
              ReliableConnection::RdmaWrite,
              quorum::pack(quorum::EntryWr, i, req_id), address,
              static_cast<uint32_t>(size),
              eventualSender->ce->connections().at(i).remoteBuf() + offset);
        }
      }
      auto res = response(request, ResponseStatus::NoError, debug);
      return res;
    }
    // conflicting call
    else {
      if (unlikely(debug)) {
        repl_object->toString();
        for(int i = 0; i < repl_object->num_methods; i++){
          std::cout << i << " = {";
          for(size_t x = 0; x < num_process; x++){
            if(x == num_process - 1)
              std::cout << calls_applied[i][x];
            else
              std::cout << calls_applied[i][x] << ",";
          }
          std::cout << "}" << std::endl;
        }
      }
      dory::ProposeError err =
          synchSenders[synch_gp]->propose(payload, payload_buffer.size());
      if (err != dory::ProposeError::NoError) {
        // request dropping, erasing it...
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
                      << synchSenders[synch_gp]->potentialLeader() << std::endl;
            break;

          default:
            std::cout << "Bug in code. You should only handle errors here"
                      << std::endl;
        }
        return response(request, ResponseStatus::DoryError, false);
      }
      // executed with no error - sending the response.
      return response(request, ResponseStatus::NoError, debug);
    }
  }
};

void NB_Wellcoordination::readLogParts(BE_Broadcast& sender, size_t i) {
  while (sender.partIter.hasNext(i)) {
    ParsedCall pslot = ParsedCall(sender.partIter.location(i));
    auto [buf, len] = pslot.payload();
    if (!pslot.isPopulated()) break;
    MethodCall* call = callFactory.deserialize(buf);
    repl_object->execute(*call);
    calls_applied[call->method_type][i]++;
    sender.partIter.next(i);
  }
  return;
}

void NB_Wellcoordination::executeOrBlock(MethodCall call, bool leader, int l) {
  while (!leader && !checkCallDependencies(call));
  repl_object->execute(call); 
  calls_applied[call.method_type][0]++;
  return;
}

bool NB_Wellcoordination::checkCallDependencies(MethodCall callWithDeps) {
  if (repl_object->dependency_relation.find(callWithDeps.method_type) == repl_object->dependency_relation.end()) return true;
  for (size_t x = 0; x < repl_object->dependency_relation[callWithDeps.method_type].size(); x++) {
    int dependency_method = repl_object->dependency_relation[callWithDeps.method_type][x];
    // std::cout << "applied " << dependency_method << " = {";
    for (size_t i = 0; i < num_process; i++){
      // if(i == num_process - 1)
      //   std::cout << calls_applied[dependency_method][i];
      // else
      //   std::cout << calls_applied[dependency_method][i] << ",";
      if (calls_applied[dependency_method][i] <
          callWithDeps.dependency_vectors[x][i])
        return false;
    }
    // std::cout << "}" << std::endl;
  }
  return true;
}
