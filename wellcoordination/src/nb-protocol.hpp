// #include "synchronizer.hpp"
// #include "readerwriterqueue.h"

// using namespace hamsaz;

// class NB_Wellcoordination : Synchronizer {
//   //  private:
//  public:
//   BankAccount account = BankAccount(1000000);
//   int pre_state;
//   int withdrawn_amount = 0;
//   int self;
//   std::atomic<int> local_state = 0;
//   std::atomic<int> receive_states = 0;

//   std::vector<int> remote_ids;
//   size_t num_process;
//   uint64_t req_id = 0;
//   std::vector<ptrdiff_t> partitions_offset;
//   std::unique_ptr<BE_Broadcast> eventualSender;
//   std::unique_ptr<dory::Consensus> synchSender;
//   std::atomic<int>* deposits_applied;
//   int withdraws_applied = 0;

//   uint64_t throughput_end;

//   moodycamel::ReaderWriterQueue<BankAccountCall> request_queue;

//   std::vector<BankAccountCall> pending_dependencies;
//   std::set<size_t> executed_pending_index;
  
//   bool executeOrAddPendingDependencies(BankAccountCall call);
//   bool checkCallDependencies(BankAccountCall callWithDeps);

//   //  public:
//   std::unordered_map<std::string, uint64_t> query_response_time;
//   std::unordered_map<std::string, uint64_t> deposit_response_time;
//   std::unordered_map<std::string, uint64_t> withdraw_response_time;
//   NB_Wellcoordination(int id, std::vector<int> remote_ids);
//   ~NB_Wellcoordination();
//   void readLogParts(BE_Broadcast& sender, size_t i);
//   std::optional<BankAccountCall> checkPendingDependencies();
//   void execute(BankAccountCall call, size_t origin);


//   virtual void request(BankAccountCall request, bool debug,
//                                   bool summarize) {
//   std::vector<uint8_t> payload_buffer;
//   payload_buffer.resize(256);
//   uint8_t* payload = &payload_buffer[0];
//   // uint64_t start = std::chrono::duration_cast<std::chrono::microseconds>(
//   //             std::chrono::high_resolution_clock::now().time_since_epoch())
//   //             .count();

//   // non-conflicting calls
//   // no need to defer the permissibility check
//   // can execute in-place (however, check for the permissibility right away
//   // before execution)
//   if (request.method_name.compare("deposit") == 0) {
//     if (unlikely(debug)) {
//       for (size_t i = 0; i < num_process; i++)
//         std::cout << deposits_applied[i] << ", ";
//       std::cout << std::endl;
//     }
//     // deposit_response_time[idStr] = start;
//     // invariant sufficient call
//     // always permissibile
//     // execute locally
//     // read your own writes style
//     account.balance += request.arg;
//     deposits_applied[self - 1]++;
//     // execute(request, self - 1);
    
    
//     size_t length = 0;
//     // if(!summarize)
//     length = request.serialize(payload);
//     // else
//     // {
//     //   BankAccountCall newPart =
//     //     BankAccountCall(request.id, "deposit", std::to_string(local_state));
//     //   length = newPart.serialize(payload);
//     // }
//     payload_buffer.resize(length);
//     Call call(*(eventualSender->log.get()), partitions_offset[self - 1],
//               payload, length, summarize);
//     auto [address, offset, size] = call.location();

//     for (auto& i : remote_ids) {
//       auto pid = i - 1;

//       // std::cout << "writing " << payload << " to: " << i << " at: " <<
//       // std::hex
//       //           << eventualSender->ce->connections().at(i).remoteBuf() +
//       //           offset
//       //           << std::endl;
//       auto ok = eventualSender->ce->connections().at(i).postSendSingleCached(
//           ReliableConnection::RdmaWrite,
//           quorum::pack(quorum::EntryWr, i, req_id++), address,
//           static_cast<uint32_t>(size),
//           eventualSender->ce->connections().at(i).remoteBuf() + offset);
//     }
//     eventualSender->pollCQ();

//     response(request, 1, debug);
//   } else if (request.method_name.compare("query") == 0) {
//     // query_response_time[idStr] = start;
//     // int res = local_state;
//     // for (auto& i : remote_ids) {
//     // std::cout << "reading from " << i << std::endl;
//     // update balance;
//     // BankAccountCall c = readLogParts(*eventualSender.get(), i - 1);
//     // res += std::stoi(c.args[0]);
//     // res += account.balance;
//     // }
//     // res -= withdrawn_amount;
//     account.balance += receive_states;
//     response(request, account.balance, debug);
//   } else if (request.method_name.compare("withdraw") == 0) {
//     if (unlikely(debug)) {
//       std::cout << "withdrawals: " << withdraws_applied << std::endl;
//       std::cout << "executed pendings size : " << executed_pending_index.size() << std::endl;
//       std::cout << "pendings size : " << pending_dependencies.size() << std::endl;
//     }
//     // if(idStr == "warmup"){
//     //   synchSender->propose(payload, payload_buffer.size());
//     //   synchSender->propose(payload, payload_buffer.size());
//     // }
//     if (request.isPermissible(account)) {
//       // withdraw_response_time[idStr] = start;
//       int* ds = new int[num_process];
//       for(size_t i = 0; i < num_process; i++)
//         ds[i] = deposits_applied[i].load();
//       request.setDeps(ds, num_process);
//       auto length = request.serialize(payload);
//       payload_buffer.resize(length);

//       dory::ProposeError err =
//           synchSender->propose(payload, payload_buffer.size());
//       response(request, 1, false);
//       if (err != dory::ProposeError::NoError) {
//         switch (err) {
//           case dory::ProposeError::FastPath:
//           case dory::ProposeError::FastPathRecyclingTriggered:
//           case dory::ProposeError::SlowPathCatchFUO:
//           case dory::ProposeError::SlowPathUpdateFollowers:
//           case dory::ProposeError::SlowPathCatchProposal:
//           case dory::ProposeError::SlowPathUpdateProposal:
//           case dory::ProposeError::SlowPathReadRemoteLogs:
//           case dory::ProposeError::SlowPathWriteAdoptedValue:
//           case dory::ProposeError::SlowPathWriteNewValue:
//             std::cout << "Error: in leader mode. Code: "
//                       << static_cast<int>(err) << std::endl;
//             break;

//           case dory::ProposeError::SlowPathLogRecycled:
//             std::cout << "Log recycled, waiting a bit..." << std::endl;
//             std::this_thread::sleep_for(std::chrono::seconds(1));
//             break;

//           case dory::ProposeError::MutexUnavailable:
//           case dory::ProposeError::FollowerMode:
//             std::cout << "Error: in follower mode. Potential leader: "
//                       << synchSender->potentialLeader() << std::endl;
//             break;

//           default:
//             std::cout << "Bug in code. You should only handle errors here"
//                       << std::endl;
//         }
//       }
//     } else
//       std::cout << "not permissible, dropping request" << std::endl;
//   } else {
//     std::cout << "invalid request " << request.call << std::endl;
//     return;
//   }
//   return;
// }

// virtual void response(BankAccountCall request, int response,
//                                           bool debug) {
//   auto end = std::chrono::duration_cast<std::chrono::microseconds>(
//                  std::chrono::high_resolution_clock::now().time_since_epoch())
//                  .count();
//   if (request.method_name == "query")
//     query_response_time[request.id] = (end - query_response_time[request.id]);
//   else if (request.method_name == "deposit")
//     deposit_response_time[request.id] = (end - deposit_response_time[request.id]);
//   else
//     withdraw_response_time[request.id] = (end - withdraw_response_time[request.id]);
//   if (debug)
//     std::cout << "response for " << request.id << ": " << response << std::endl;
// }

// };

// NB_Wellcoordination::NB_Wellcoordination(int id, std::vector<int> r_ids) {
//   self = id;
//   remote_ids = r_ids;
//   num_process = remote_ids.size() + 1;
//   partitions_offset.resize(num_process);
//   pre_state = account.balance;
//   deposits_applied = new std::atomic<int>[8];

//   request_queue = moodycamel::ReaderWriterQueue<BankAccountCall>(128);
//   for (size_t i = 0; i < num_process; i++) {
//     deposits_applied[i] = 0;
//     // withdraws_applied[i] = 0;
//   }

//   // each partition has an offset of: (LogConfig::LOG_SIZE/num_process)*part_num
//   for (size_t i = 0; i < num_process; i++) {
//     partitions_offset[i] =
//         LogConfig::round_up_powerof2(LogConfig::LOG_SIZE / num_process) * i;
//   }
//   eventualSender = std::make_unique<BE_Broadcast>(id, remote_ids);
//   synchSender = std::make_unique<dory::Consensus>(id, remote_ids, 16,
//                                                   dory::ThreadBank::A);
//   // std::this_thread::sleep_for(std::chrono::seconds(8-id));

//   // this is only called in the followers
//   // in the leader, after proposal we directly call response
//   // since we know that it can be delivered to the leader right away
//   // ie., call to the handler and return of the propose method are equivalent
//   synchSender->commitHandler([&]([[maybe_unused]] bool leader,
//                                  [[maybe_unused]] uint8_t* buf,
//                                  [[maybe_unused]] size_t len) {
    
//     BankAccountCall response = BankAccountCall::deserialize(buf);
//     // std::string idStr(response.id, response.id + response.id_len);
//     // if(unlikely(idStr == "warmup"))
//     //   return;
//     // execute conflicting call (add dependency check here)
//     int temp = 0;
//     if(!executeOrAddPendingDependencies(response)){
//       // std::cout << response.id << " queued for later exec" << std::endl;
//     }
//     checkPendingDependencies();
//     // withdrawn_amount += response.arg;
//   });

  

//   std::thread logReader([&] {
//     while(true)
//     {
//       int temp = 0;
//       for (auto& i : remote_ids)
//         temp += readLogParts(*eventualSender.get(), i-1);
//       receive_states = temp;
//       // account.balance = pre_state + receive_states + local_state - withdrawn_amount;
//       // need to wait here otherwise writes are not done from remote and
//       // the results are not correct
//       std::this_thread::sleep_for(std::chrono::microseconds(200));
//     }
//     return;
//   });
  
//   // std::thread runner([&] {
//   //   while(true){
//   //     BankAccountCall call = BankAccountCall();
//   //     if(request_queue.try_dequeue(call)){
//   //         std::vector<uint8_t> payload_buffer;
//   //         payload_buffer.resize(256);
//   //         uint8_t* payload = &payload_buffer[0];
//   //         call.serialize(payload);
//   //         BankAccountCall newCall = BankAccountCall();
//   //         newCall.deserialize(payload);
//   //         newCall.toString();
//   //         // request_queue.pop();

//   //       request(call, false, false);
//   //       // break;
//   //     }
//   //   }
//   // });
//   // runner.detach();
//   logReader.detach();

//   // this causes data-race because withdrawn_amount is updated in multiple threads
//   // std::thread depsCheck([&] {
//   //   while (true) {
//   //     checkPendingDependencies();
//   //     std::this_thread::sleep_for(std::chrono::microseconds(50));
//   //   }
//   // });
//   // depsCheck.detach();
  
// }

// NB_Wellcoordination::~NB_Wellcoordination() { eventualSender.reset(); }

// void NB_Wellcoordination::execute(BankAccountCall call, size_t origin){
//   if(call.method_name == "deposit"){
//     local_state += call.arg;
//     deposits_applied[origin]++;
//   }
//   else if(call.method_name == "withdraw"){
//     withdrawn_amount += call.arg;
//     withdraws_applied++;
//   }

// }

// int NB_Wellcoordination::readLogParts(BE_Broadcast& sender, size_t i) {
//   int aggr_remote_states = 0;
//   while (sender.partIter.hasNext(i)) {
//     ParsedCall pslot = ParsedCall(sender.partIter.location(i));
//     auto [buf, len] = pslot.payload();
//     if (!pslot.isPopulated()) break;
//     BankAccountCall call = BankAccountCall::deserialize(buf);
//     // received one deposit from remote i
//     aggr_remote_states += call.arg;
//     deposits_applied[i]++;
//     // execute(call, i);
//     sender.partIter.next(i);
//   }
//   // sender.partIter.reset(i);
//   // aggr_remote_states += arg;
//   return aggr_remote_states;
// }

// bool NB_Wellcoordination::executeOrAddPendingDependencies(
//     BankAccountCall call) {
//   if (checkCallDependencies(call)) {
//     execute(call, 0);
//     return true;
//   } else {
//     pending_dependencies.push_back(call);
//     return false;
//   }
// }

// bool NB_Wellcoordination::checkCallDependencies(BankAccountCall callWithDeps) {
//   if (callWithDeps.deps == NULL) return true;
//   for (size_t i = 0; i < num_process; i++) {
//     if (deposits_applied[i] < callWithDeps.deps[i]) return false;
//   }
//   return true;
// }

// int NB_Wellcoordination::checkPendingDependencies() {
//   int executed = 0;
//   for (size_t i = 0; i < pending_dependencies.size(); i++) {
//     if(executed_pending_index.find(i) == executed_pending_index.end()){
//       BankAccountCall pending = pending_dependencies[i];
//       // std::cout << "checking " << pending.id << " with ";
//       // for (size_t i = 0; i < num_process; i++)
//         // std::cout << pending.deps[i] << ", ";
//       // std::cout << std::endl; 
//       if (checkCallDependencies(pending)) {
//         // std::cout << pending.id << " found in queue, executing..." << std::endl;
//         executed_pending_index.insert(i);
//         // executing a pending call
//         execute += pending.arg;
//         withdraws_applied++;
//       }
//     }
//   }
//   return executed;
// }
