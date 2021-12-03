#include <thread>

#include <dory/crash-consensus.hpp>
#include <dory/shared/branching.hpp>

#include "synchronizer.hpp"

class Mu : Synchronizer {
  //  private:
 public:
  // common in all the use-cases
  int self;
  size_t num_process;
  std::vector<int> remote_ids;
  uint64_t req_id = 0;
  std::unique_ptr<dory::Consensus> synchSenders;
  MethodCallFactory callFactory;
  uint64_t throughput_end;
  std::atomic<int>** calls_applied;
  uint64_t readDelay = 0;

  ReplicatedObject* repl_object;

  const uint64_t QUEUE_SIZE = 10000;

  ~Mu() {}

  Mu(int id, std::vector<int> r_ids, ReplicatedObject* obj) {
    self = id;
    remote_ids = r_ids;
    num_process = remote_ids.size() + 1;
    repl_object = obj;
    callFactory = MethodCallFactory(repl_object, num_process);
    calls_applied = new std::atomic<int>*[repl_object->num_methods];
    for (int i = 0; i < repl_object->num_methods; i++)
      calls_applied[i] = new std::atomic<int>[num_process];
    for (int x = 0; x < repl_object->num_methods; x++)
      for (size_t i = 0; i < num_process; i++) calls_applied[x][i] = 0;
    
    synchSenders = std::make_unique<dory::Consensus>(id, remote_ids, 16,
                                                     dory::ThreadBank::A);

    // this is only called in the followers
    // in the leader, after proposal we directly call response
    // since we know that it can be delivered to the leader right away
    // ie., call to the handler and return of the propose method are equivalent
    synchSenders->commitHandler([&]([[maybe_unused]] bool leader,
                                    [[maybe_unused]] uint8_t* buf,
                                    [[maybe_unused]] size_t len) {
      MethodCall* response = callFactory.deserialize(buf);
      // std::cout << "received call" << std::endl;
      repl_object->execute(*response);
      calls_applied[response->method_type][0]++;
    });
  }

   virtual std::pair<ResponseStatus,std::chrono::high_resolution_clock::time_point> request(MethodCall request, bool debug, bool summarize) {
    // a query method
    // handle localy and do not propagate
    if (request.method_type == repl_object->read_method) 
      return response(request, ResponseStatus::NoError, false);

    
    std::vector<uint8_t> payload_buffer(256);
    uint8_t* payload = &payload_buffer[0];

    if (!repl_object->isPermissible(request)) {
      std::cout << "not permissible, dropping request" << std::endl;
      return response(request, ResponseStatus::NotPermissible, debug);
    }
    // serialize the call
    auto length = callFactory.serialize(request, payload);
    payload_buffer.resize(length);
    // propose the call to others
    dory::ProposeError err =
        synchSenders->propose(payload, payload_buffer.size());

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
          std::cout << "Error: in leader mode. Code: " << static_cast<int>(err)
                    << std::endl;
          break;

        case dory::ProposeError::SlowPathLogRecycled:
          std::cout << "Log recycled, waiting a bit..." << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds(1));
          break;

        case dory::ProposeError::MutexUnavailable:
        case dory::ProposeError::FollowerMode:
          // std::cout << "Error: in follower mode. Potential leader: "
          //           << synchSenders->potentialLeader() << std::endl;
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
};