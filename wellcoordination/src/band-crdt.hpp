#include <thread>

#include <dory/shared/branching.hpp>

#include "rb-crdt.hpp"
#include "synchronizer.hpp"

using namespace band;

class BandCRDT {
 public:
  int self;
  size_t num_process;
  std::vector<int> remote_ids;
  std::unique_ptr<ReliableBroadcast> rb;
  ReplicatedObject* repl_object;

  std::vector<uint8_t> payload_buffer;
  uint8_t* payload;

  ~BandCRDT() { rb.reset(); }

  BandCRDT(int id, std::vector<int> r_ids, ReplicatedObject* obj) {
    self = id;
    remote_ids = r_ids;
    num_process = remote_ids.size() + 1;
    repl_object = obj;
    payload_buffer.resize(512);
    payload = &payload_buffer[0];
    
    rb = std::make_unique<ReliableBroadcast>(id, remote_ids, repl_object);
  }

  ReplicatedObject* request(MethodCall request, bool debug, bool summarize) {
    // a query method
    // handle localy and do not propagate
    if(std::find(repl_object->read_methods.begin(), repl_object->read_methods.end(), request.method_type) != repl_object->read_methods.end())
      return repl_object;
    
    // local at source + downstream source   
    std::string newArg = repl_object->internalExecuteCRDT(request, self - 1);
    // std::cout << "arg: " << request.arg << std::endl;
    // std::cout << "new arg: " << newArg << std::endl;
    request.arg += "-" + newArg;
    repl_object->internalDownstreamExecuteCRDT(request, self - 1, false);
    auto length = repl_object->serializeCRDT(request, payload);
    payload_buffer.resize(length);
      
    //reliable broadcast
    rb->broadcast(payload, length, summarize);

    return repl_object;
  }
};
