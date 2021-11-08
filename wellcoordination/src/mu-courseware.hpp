#include <thread>

#include <dory/crash-consensus.hpp>
#include <dory/shared/branching.hpp>

#include "courseware.hpp"
#include "synchronizer.hpp"
#include <mutex>

// using namespace hamsaz;

class Mu : Synchronizer {
  //  private:
 public:
  // common in all the use-cases
  int self;
  size_t num_process;
  std::vector<int> remote_ids;
  uint64_t req_id = 0;
  std::unique_ptr<dory::Consensus> synchSenders;
  std::unordered_map<std::string, uint64_t>* response_times;
  std::atomic<int>** calls_applied;
  MethodCallFactory callFactory;
  uint64_t throughput_end;

  Courseware courseware = Courseware();

  const uint64_t QUEUE_SIZE = 10000;

  ~Mu(){
    delete[] response_times;
    delete[] calls_applied;
  }

  Mu(int id, std::vector<int> r_ids) {
    self = id;
    remote_ids = r_ids;
    num_process = remote_ids.size() + 1;
    callFactory = MethodCallFactory(courseware, num_process);
    calls_applied = new std::atomic<int>*[courseware.num_methods];
    for (int i = 0; i < courseware.num_methods; i++)
      calls_applied[i] = new std::atomic<int>[num_process];
    for (int x = 0; x < courseware.num_methods; x++)
      for (size_t i = 0; i < num_process; i++) calls_applied[x][i] = 0;

    
    response_times = new std::unordered_map<std::string, uint64_t>[courseware.num_methods];
    for(int i = 0; i < courseware.num_methods; i++)
      response_times[i] = std::unordered_map<std::string,uint64_t>();

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
      // std::cout << "received call" << std::endl;
      // callFactory.toString(response);
      courseware.execute(response); 
      calls_applied[response.method_type][leader - 1]++;
    });
  }

  virtual void request(std::string input, bool debug, bool summarize) {
    std::vector<uint8_t> payload_buffer(256);
    uint8_t* payload = &payload_buffer[0];
    std::string sequence_number = std::to_string(self) + "-" + std::to_string(req_id++);
    MethodCall request = callFactory.createCall(sequence_number, input);
    response_times[request.method_type][sequence_number] = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch())
                .count();;
    // a query method
    // handle localy and do not propagate
    if (std::find(courseware.update_methods.begin(), courseware.update_methods.end(), request.method_type) == courseware.update_methods.end()) {
      if(debug)
        courseware.toString();
      response(request, 1, debug);
      return;
    }
    else {
      if (!courseware.isPermissible(request)) {
        std::cout << "not permissible, dropping request" << std::endl;
        return;
      }
      // serialize the call
      auto length = callFactory.serialize(request, payload);
      payload_buffer.resize(length);
      // propose the call to others
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
      std::cout << "response time " << response_times[request.method_type][request.id] << std::endl;
  }
};