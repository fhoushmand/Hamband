// #include <stdlib.h>
// #include <cstdio>
// #include <fstream>
// #include <iostream>
// #include <limits>
// #include <string>

// #include <dory/store.hpp>

// #include "band-crdt.hpp"

// #include "../config.h"

// #include "../benchmark/counter-crdt.hpp"
// #include "../benchmark/orset-crdt.hpp"

// int main(int argc, char* argv[]) {
//   if (argc < 2) {
//     throw std::runtime_error("Provide the id of the process as argument");
//   }
//   constexpr int minimum_id = 1;
//   std::string idstr(argv[1], argv[1] + 1);
//   int id = std::stoi(idstr);
//   int nr_procs = static_cast<int>(std::atoi(argv[2]));
//   int num_ops = static_cast<int>(std::atoi(argv[3]));
//   double write_percentage = static_cast<double>(std::atoi(argv[4]));
//   std::string usecase = std::string(argv[5]);
//   bool calculate_throughput = (std::atoi(argv[6]) == 1);
//   double total_writes = num_ops * (write_percentage / 100);

//   std::cout << "number of operations: " << num_ops << std::endl;
//   std::cout << "write precentage: "
//             << static_cast<double>(write_percentage / 100) << std::endl;
//   std::string loc = WORKLOAD_LOCATION;
//   loc += std::to_string(nr_procs) + "-" + std::to_string(num_ops) + "-" +
//          std::to_string(static_cast<int>(write_percentage));
//   loc += "/" + usecase + "/";
  
//   // Build the list of remote ids
//   std::vector<int> remote_ids;
//   for (int i = 0, min_id = minimum_id; i < nr_procs; i++, min_id++) {
//     if (min_id == id) {
//       continue;
//     } else {
//       remote_ids.push_back(min_id);
//     }
//   }

//   ReplicatedObject* object = NULL;
//   if(usecase == "counter") {
//     object = new Counter();
//   }
//   else if(usecase == "orset") {
//     object = new ORSet();
//   }
//   object->setID(id)->setNumProcess(nr_procs)->finalize();
  
//   std::unordered_map<std::string, uint64_t>* response_times =
//       new std::unordered_map<std::string, uint64_t>[object->num_methods];
//   for (int i = 0; i < object->num_methods; i++)
//     response_times[i] = std::unordered_map<std::string, uint64_t>();

//   auto& store = dory::MemoryStore::getInstance();
//   NB_Wellcoordination protocol(id, remote_ids, object);
//   std::this_thread::sleep_for(std::chrono::seconds(10));

//   int call_id = 0;
//   int new_sent = 0;
//   std::string l;
//   std::atomic<bool> own_requests_done = false;
//   std::ifstream node_requests;
//   std::thread requestRedirector;
//     node_requests.open((loc + "redirect" + ".txt").c_str());
//     requestRedirector = std::thread([&] {
//       while (true) {
//         if (own_requests_done.load()) {
//           std::cout << "started sending remaining follower messages" << std::endl;
//           while (getline(node_requests, l)) {
//             std::string sequence_number =
//             std::to_string(id) + "-" + std::to_string(call_id++);
//             MethodCall call = ReplicatedObject::createCall(sequence_number, l);
//             protocol.request(call, false, false);
//             new_sent++;
//           }
//           std::cout << "finished sending" << std::endl;
//           break;
//         }
//       }
//       return;
//     });
  

  

//   if(id != 1)
//     std::this_thread::sleep_for(std::chrono::milliseconds(5));
//   store.set(std::to_string(id), "ready");
//   for(int i = 1; i < nr_procs; i++)
//   {
//     std::string value;
//     while (!store.get(std::to_string(i), value));
//   }

//   std::cout << "started sending..." << std::endl;

//   uint64_t local_start = std::chrono::duration_cast<std::chrono::microseconds>(
//                    std::chrono::high_resolution_clock::now().time_since_epoch()).count();
//   store.set(std::to_string(id), std::to_string(local_start));

//   int sent = 0;
//   std::string line;
//   int expected_calls = 0;
//   std::ifstream myfile;
//   if(id == 1)
//     myfile.open((loc + std::to_string(id) + "-failure" + ".txt").c_str());
//   else
//     myfile.open((loc + std::to_string(id) + ".txt").c_str());


//   if(calculate_throughput) {
//     // start reading from the request file
//     while (getline(myfile, line)) {
//     // for(std::string line : requests) {
//       if (unlikely(line.at(0) == '#')) {
//         expected_calls = std::stoi(line.substr(1, line.size()));
//         continue;
//       }
//       // failure
//       if (unlikely(id == 1 && line.at(0) == 'X')) {
//         std::cout << "stoping heartbeat thread and waiting..." << std::endl;
//         protocol.rb->hb_active.store(false);
//         std::this_thread::sleep_for(std::chrono::seconds(60));
//         break;
//       }
//       else
//         if (unlikely(line.at(0) == 'X')) continue;

//       std::string sequence_number =
//           std::to_string(id) + "-" + std::to_string(call_id++);
//       MethodCall call = ReplicatedObject::createCall(sequence_number, line);
//       // calculating the response time
      
//       std::pair<ResponseStatus, std::chrono::high_resolution_clock::time_point>
//           response = protocol.request(call, false, false);
//       sent++;
//     }
//   }
  
//   else {
//     // start reading from the request file
//     while (getline(myfile, line)) {
//       if (unlikely(line.at(0) == '#')) {
//         expected_calls = std::stoi(line.substr(1, line.size()));
//         continue;
//       }
//       // failure
//       if (unlikely(id == 1 && line.at(0) == 'X')) {
//         std::cout << "stoping heartbeat thread and waiting..." << std::endl;
//         protocol.rb->hb_active.store(false);
//         std::this_thread::sleep_for(std::chrono::seconds(60));
//         break;
//       }
//       else
//         if (unlikely(line.at(0) == 'X')) continue;

//       std::string sequence_number =
//           std::to_string(id) + "-" + std::to_string(call_id++);
//       MethodCall call = ReplicatedObject::createCall(sequence_number, line);
//       // calculating the response time
//       auto start = std::chrono::high_resolution_clock::now();
//       std::pair<ResponseStatus, std::chrono::high_resolution_clock::time_point>
//           response = protocol.request(call, false, false);
//       if (response.first == ResponseStatus::NoError) {
//         sent++;
//         response_times[call.method_type][call.id] =
//             std::chrono::duration_cast<std::chrono::nanoseconds>(response.second -
//                                                                   start)
//                 .count();
//       }
//     }
//   }

//   uint64_t local_end = std::chrono::duration_cast<std::chrono::microseconds>(
//                    std::chrono::high_resolution_clock::now().time_since_epoch()).count();

//   std::cout << "issued " << sent << " operations" << std::endl;


//   if(id == 2){
//     own_requests_done.store(true);
//     requestRedirector.join();
//     std::cout << "new sent: " << new_sent << std::endl;
//   }

//   if(!calculate_throughput){
//     double sum = 0;
//     double total_sum = 0;
//     size_t num = 0;
//     for (int i = 0; i < object->num_methods; i++) {
//       total_sum += sum;
//       sum = 0;
//       for (auto& pair : response_times[i]){
//         sum += static_cast<double>(pair.second);
//         num++;
//       }
//       std::cout << "average response time for " << response_times[i].size()
//                 << " calls to " << i << ": "
//                 << (sum/1000) / static_cast<int>(response_times[i].size()) << std::endl;
//     }
//     std::cout << "total average response time for " << num
//               << " calls: " << (total_sum/1000) / static_cast<int>(num) << std::endl;
//   }

//   // wait for all the ops to arrive and then calculate throughput
//   int cs = 0;
//   int sz = static_cast<int>(object->synch_groups.size());
//   while (true) {
//     cs = 0;
//     for (int i = 0; i < object->num_methods; i++){
//       for (int x = 0; x < nr_procs; x++) {
//         cs += protocol.repl_object->calls_applied[i][x];
//       }
//     }
//     // std::cout << "received: " << cs << std::endl;
//     if (cs == expected_calls)
//       break;
//     std::this_thread::sleep_for(std::chrono::microseconds(10));
//   }

//   uint64_t global_end = std::chrono::duration_cast<std::chrono::microseconds>(
//                    std::chrono::high_resolution_clock::now().time_since_epoch()).count();

//   if(calculate_throughput)
//     std::cout << "throughput:"
//             << static_cast<double>(num_ops)/static_cast<double>(global_end - local_start) << std::endl;
//   // object->toString();

//   std::this_thread::sleep_for(std::chrono::seconds(60));
//   return 0;
// }