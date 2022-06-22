#include <thread>

#include <dory/crash-consensus.hpp>
#include <dory/shared/branching.hpp>

#include "beb.hpp"
#include "synchronizer.hpp"


using namespace band;

class NB_Wellcoordination : Synchronizer {
 public:
  int self;
  size_t num_process;
  std::vector<int> remote_ids;
  std::unique_ptr<ReliableBroadcast> rb;
  std::unique_ptr<dory::Consensus>* tob;
  ReplicatedObject* repl_object;

  void executeOrBlock(MethodCall call, bool leader, int l);
  bool checkCallDependencies(MethodCall callWithDeps);

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
        executeOrBlock(request, leader, static_cast<int>(i));
      });
    }
  }

  virtual std::pair<ResponseStatus,std::chrono::high_resolution_clock::time_point> request(MethodCall request, bool debug, bool summarize) {
    // a query method
    // handle localy and do not propagate
    // std::cout << "request: " << request.id << std::endl;
    // std::cout << "read_methods: " << repl_object->read_methods.size() << std::endl;
    if(std::find(repl_object->read_methods.begin(), repl_object->read_methods.end(), request.method_type) != repl_object->read_methods.end())
    {
      return response(request, ResponseStatus::NoError, false);
    }

    std::vector<uint8_t> payload_buffer(256);
    uint8_t* payload = &payload_buffer[0];

    // std::cout << "serialize" << request.method_type << std::endl;
    auto length = repl_object->serialize(request, payload);
    payload_buffer.resize(length);
    int synch_gp = repl_object->getSynchGroup(request.method_type);
    
    if (!repl_object->isPermissible(request)) {
        std::cout << "not permissible, dropping request " << request.method_type << std::endl;
        return response(request, ResponseStatus::NotPermissible, debug);
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
      dory::ProposeError err =
          tob[synch_gp]->propose(payload, payload_buffer.size());
      // std::cout << "after propose" << std::endl;
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
                      << tob[synch_gp]->potentialLeader() << std::endl;
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

void NB_Wellcoordination::executeOrBlock(MethodCall call, bool leader, int l) {
  while (!leader && !checkCallDependencies(call));
  repl_object->internalExecute(call, 0); 
  return;
}

bool NB_Wellcoordination::checkCallDependencies(MethodCall callWithDeps) {
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
