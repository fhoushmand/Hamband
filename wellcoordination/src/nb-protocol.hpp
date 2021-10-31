
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>

#include <dory/crash-consensus.hpp>
#include "../usecase/account/bank_account.hpp"
#include "beb.hpp"
#include "timers.h"


namespace hamsaz {

class NB_Wellcoordination {
 private:
  BankAccount account = BankAccount(0);
  int withdrawn_amount = 0;
  int self;
  uint64_t start = 0;
  int local_state = 0;
  std::vector<int> remote_ids;
  size_t num_process;
  uint64_t req_id = 0;
  uint64_t sum_response_time = 0;
  std::vector<ptrdiff_t> partitions_offset;
  std::unique_ptr<BE_Broadcast> eventualSender;
  std::unique_ptr<dory::Consensus> synchSender;
  int* deposits_applied;
  int* withdraws_applied;
  
  std::vector<std::pair<BankAccountCall,int*>> pending_dependencies;
  bool executeOrAddPendingDependencies(BankAccountCall call, int* deps);
  bool checkCallDependencies(std::pair<BankAccountCall,int*> callWithDeps);

 public:
  std::unordered_map<std::string,uint64_t> response_time;
  NB_Wellcoordination(int id, std::vector<int> remote_ids);
  ~NB_Wellcoordination();
  BankAccountCall readLogParts(BE_Broadcast& sender, size_t i);
  void request(BankAccountCall req, bool debug, bool summarize);
  inline void response(BankAccountCall req, int res, bool debug);
  std::optional<BankAccountCall> checkPendingDependencies();
};

NB_Wellcoordination::NB_Wellcoordination(int id, std::vector<int> r_ids) {
  self = id;
  remote_ids = r_ids;
  num_process = remote_ids.size() + 1;
  partitions_offset.resize(num_process);

  deposits_applied = new int[num_process];
  withdraws_applied = new int[num_process];


  // each partition has an offset of: (LogConfig::LOG_SIZE/num_process)*part_num
  for (size_t i = 0; i < num_process; i++) {
    partitions_offset[i] =
        LogConfig::round_up_powerof2(LogConfig::LOG_SIZE / num_process) * i;
  }
  eventualSender = std::make_unique<BE_Broadcast>(id, remote_ids);
  synchSender = std::make_unique<dory::Consensus>(id, remote_ids, 10,
                                                  dory::ThreadBank::A);

  // this is only called in the followers
  // in the leader, after proposal we directly call response
  // since we know that it can be delivered to the leader right away
  // ie., call to the handler and return of the propose method are equivalent
  synchSender->commitHandler([&]([[maybe_unused]] bool leader,
                                 [[maybe_unused]] uint8_t* buf,
                                 [[maybe_unused]] size_t len) {
    std::string call(buf, buf + len);
    BankAccountCall response = BankAccountCall::fromStr(call);
    // execute conflicting call
    withdrawn_amount += std::stoi(response.args[0]);

    auto end = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch())
                   .count();
    // print only the ones that this replica has not sent
    if(response.id.rfind(std::to_string(self), 0) != 0)
    {
      // std::cout << "received call to " << call << std::endl;
      // response_time[response.id] = (end - response_time[response.id]);
      // std::cout << std::dec << response_time[response.id] << " micro" << std::endl;
    }
    
  });

  std::thread logReader([&] {
    while(true)
      for (auto& i : remote_ids)
        readLogParts(*eventualSender.get(), i-1);
    return;
  });
  logReader.detach();
  
}

NB_Wellcoordination::~NB_Wellcoordination() { eventualSender.reset(); }

BankAccountCall NB_Wellcoordination::readLogParts(BE_Broadcast& sender,
                                                  size_t i) {
  int arg = 0;
  while (sender.partIter.hasNext(i)) {
    ParsedCall pslot = ParsedCall(sender.partIter.location(i));
    auto [buf, len] = pslot.payload();
    if (!pslot.isPopulated()) break;
    std::string callS(buf, buf + len);
    // std::cout << "call: " << callS << std::endl;
    // std::cout << "buf: " << buf << std::endl;
    // std::cout << "len: " << len << std::endl;
    BankAccountCall call = BankAccountCall::fromStr(callS);
    arg += std::stoi(call.args[0]);
    // received one deposit from remote i
    deposits_applied[i]++;
    if (pslot.hasDeps()) {
      printf("deps: %d, %d, %d", pslot.dependencies()[0],
             pslot.dependencies()[1], pslot.dependencies()[2]);
      std::cout << std::endl;
    }
    sender.partIter.next(i);
  }
  // sender.partIter.reset(i);
  account.balance += arg;
  return BankAccountCall("0", "deposit", std::to_string(arg).c_str());
}

bool NB_Wellcoordination::executeOrAddPendingDependencies(BankAccountCall call, int* deps)
{
  if(deps == NULL || checkCallDependencies(std::make_pair(call, deps)))
  {
    withdrawn_amount += std::stoi(call.args[0]);
    return true;
  }
  else
  {
    pending_dependencies.push_back(std::make_pair(call,deps));
    return false;
  }
}

bool NB_Wellcoordination::checkCallDependencies(std::pair<BankAccountCall,int*> callWithDeps)
{
  bool hasDep = true;
  for(size_t i = 0; i < num_process; i++)
  {
    if(deposits_applied[i] < callWithDeps.second[i])
      return false;
  }
  return true;
}

std::optional<BankAccountCall> NB_Wellcoordination::checkPendingDependencies()
{
  for(size_t i = 0; i < pending_dependencies.size(); i++)
  {
    std::pair<BankAccountCall,int*> pending = pending_dependencies[i];
    if(checkCallDependencies(pending))
    {
      pending_dependencies.erase(pending_dependencies.begin() + i - 1);
      return std::optional<BankAccountCall>{pending.first};
    }
  }
  return std::nullopt;
}

void NB_Wellcoordination::request(BankAccountCall request, bool debug, bool summarize) {
  std::vector<uint8_t> payload_buffer;
  payload_buffer.resize(64);
  uint8_t* payload = &payload_buffer[0];
  
  start = std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch())
              .count();
  response_time[request.id] = start;
  // non-conflicting calls
  // no need to defer the permissibility check
  // can execute in-place (however, check for the permissibility right away before execution)
  if (request.method_name == "deposit") {
    // invariant sufficient call
    // always permissibile
    // execute locally
    local_state += std::stoi(request.args[0]);
    // executed one deposit locally
    deposits_applied[self - 1]++;
  
    size_t length = 0;
    if(!summarize)
      length = request.serialize(payload);
    else
    {
      BankAccountCall newPart =
        BankAccountCall(request.id, "deposit", std::to_string(local_state));
      length = newPart.serialize(payload);
    }
    payload_buffer.resize(length);
    Call call(*(eventualSender->log.get()), partitions_offset[self - 1], NULL,
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
  } else if (request.method_name == "query") {
    // int res = local_state;
    // for (auto& i : remote_ids) {
      // std::cout << "reading from " << i << std::endl;
      // update balance;
      // BankAccountCall c = readLogParts(*eventualSender.get(), i - 1);
      // res += std::stoi(c.args[0]);
      // res += account.balance;
    // }
    // res -= withdrawn_amount;
    response(request, account.balance + local_state - withdrawn_amount, debug);
  } else if (request.method_name == "withdraw") {
    std::cout << "executed deposits:" << std::endl;
    for(size_t i = 0; i < num_process; i++)
      std::cout << i+1 << " : " << deposits_applied[i] << std::endl;
    auto length = request.serialize(payload);
    payload_buffer.resize(length);
    dory::ProposeError err =
        synchSender->propose(payload, payload_buffer.size());
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
          std::cout << "Error: in leader mode. Code: " << static_cast<int>(err)
                    << std::endl;
          break;

        case dory::ProposeError::SlowPathLogRecycled:
          std::cout << "Log recycled, waiting a bit..." << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds(1));
          break;

        case dory::ProposeError::MutexUnavailable:
        case dory::ProposeError::FollowerMode:
          std::cout << "Error: in follower mode. Potential leader: "
                    << synchSender->potentialLeader() << std::endl;
          break;

        default:
          std::cout << "Bug in code. You should only handle errors here"
                    << std::endl;
      }
    }
  } else {
    std::cout << "invalid request!" << std::endl;
    return;
  }
  return;
}

inline void NB_Wellcoordination::response(BankAccountCall request, int response, bool debug) {
  auto end = std::chrono::duration_cast<std::chrono::microseconds>(
                 std::chrono::high_resolution_clock::now().time_since_epoch())
                 .count();
  response_time[request.id] = (end - response_time[request.id]);
  if(debug)
    std::cout << "response for " << request.id << ": " << response << std::endl;
}
}  // namespace hamsaz
