#include <stdlib.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include <dory/store.hpp>

#include "band-crdt.hpp"

#include "../config.h"

#include "../benchmark/counter-crdt.hpp"
#include "../benchmark/gset-crdt.hpp"
#include "../benchmark/orset-crdt.hpp"
#include "../benchmark/register-crdt.hpp"
#include "../benchmark/shop-crdt.hpp"
#include "../benchmark/pnset-crdt.hpp"
#include "../benchmark/twopset-crdt.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Provide the id of the process as argument");
  }
  constexpr int minimum_id = 1;
  std::string idstr(argv[1], argv[1] + 1);
  int id = std::stoi(idstr);
  int nr_procs = static_cast<int>(std::atoi(argv[2]));
  int num_ops = static_cast<int>(std::atoi(argv[3]));
  double write_percentage = static_cast<double>(std::atoi(argv[4]));
  std::string usecase = std::string(argv[5]);
  // bool calculate_throughput = (std::atoi(argv[6]) == 1);
  double total_writes = num_ops * (write_percentage / 100);

  // 0 for no failure
  // 2 for leader failure
  int failed_node = std::atoi(argv[6]);

  std::cout << "number of operations: " << num_ops << std::endl;
  std::cout << "write precentage: "
            << static_cast<double>(write_percentage / 100) << std::endl;
  std::string loc = WORKLOAD_LOCATION;
  loc += std::to_string(nr_procs) + "-" + std::to_string(num_ops) + "-" +
         std::to_string(static_cast<int>(write_percentage));
  loc += "/" + usecase + "/";
  
  // Build the list of remote ids
  std::vector<int> remote_ids;
  for (int i = 0, min_id = minimum_id; i < nr_procs; i++, min_id++) {
    if (min_id == id) {
      continue;
    } else {
      remote_ids.push_back(min_id);
    }
  }


  ReplicatedObject* object = NULL;
  if(usecase == "counter") {
    object = new Counter();
  }
  else if(usecase == "register") {
    object = new Register();
  }
  else if(usecase == "gset") {
    object = new GSet();
  }
  else if(usecase == "orset") {
    object = new ORSet();
  }
  else if(usecase == "shop") {
    object = new Shop();
  }
  else if(usecase == "pnset") {
    object = new PNSet();
  }
  else if(usecase == "twopset") {
    object = new TWOPSet();
  }
  object->setID(id)->setNumProcess(nr_procs)->finalize();
  
  std::unordered_map<std::string, uint64_t>* response_times =
      new std::unordered_map<std::string, uint64_t>[object->num_methods];
  for (int i = 0; i < object->num_methods; i++)
    response_times[i] = std::unordered_map<std::string, uint64_t>();

  auto& store = dory::MemoryStore::getInstance();
  BandCRDT protocol(id, remote_ids, object);
  std::this_thread::sleep_for(std::chrono::seconds(10));

  
  int call_id = 0;

  std::string l;
  std::vector<MethodCall> redirect_requests;
  std::ifstream failed_node_requests;
  failed_node_requests.open((loc + "redirect" + ".txt").c_str());
  while (getline(failed_node_requests, l)) 
  {
    std::string sequence_number = std::to_string(id) + "-" + std::to_string(call_id++);
    MethodCall call = ReplicatedObject::createCall(sequence_number, l);
    redirect_requests.push_back(call);
  }

  int new_sent = 0;
  std::thread requestRedirector;
  std::atomic<bool> own_requests_done = false;
  if(failed_node == 1 && id == failed_node + 1){
    requestRedirector = std::thread([&] {
      while (true) {
        if (own_requests_done.load()) {
          for (MethodCall req : redirect_requests) {
            protocol.request(req, false, false);
            new_sent++;
          }
          break;
        }
      }
      return;
    });
  }

  
  int sent = 0;
  std::string line;
  int expected_calls = 0;
  std::ifstream myfile;
  myfile.open((loc + std::to_string(id) + ".txt").c_str());
  std::cout << "fetching the requests" << std::endl;
  std::vector<MethodCall> requests;
  while (getline(myfile, line)) {
    if (unlikely(line.at(0) == 'X')) 
      break;
    if (unlikely(line.at(0) == '#')) {
      expected_calls = std::stoi(line.substr(1, line.size()));
      continue;
    }
    std::string sequence_number =
        std::to_string(id) + "-" + std::to_string(call_id++);
    MethodCall call = ReplicatedObject::createCall(sequence_number, line);
    requests.push_back(call);
  }

  if(id != 1)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  store.set(std::to_string(id), "ready");
  for(int i = 1; i < nr_procs; i++)
  {
    std::string value;
    while (!store.get(std::to_string(i), value));
  }

  std::cout << "started sending..." << std::endl;

  uint64_t local_start = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  store.set(std::to_string(id), std::to_string(local_start));

  auto start = std::chrono::high_resolution_clock::now();
  if(id == failed_node)
  {
    // start issuing the requests
    for(MethodCall call : requests) {
      if (unlikely(sent == static_cast<int>(requests.size()/2))) {
        std::cout << "stoping heartbeat thread and waiting..." << std::endl;
        protocol.rb->hb_active.store(false);
        std::this_thread::sleep_for(std::chrono::seconds(60));
        break;
      }
      protocol.request(call, false, false);
      sent++;
    }
  }
  else{
    // start issuing the requests
    for(MethodCall call : requests) {
      protocol.request(call, false, false);
      sent++;
    }
  }
  uint64_t local_end = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();

  std::cout << "issued " << sent << " operations" << std::endl;

  std::cout << "total average response time for " << sent
              << " calls: " << static_cast<double>(local_end - local_start)  / static_cast<int>(sent) << std::endl;


  if(failed_node == 1 && id == failed_node + 1){
    own_requests_done.store(true);
    requestRedirector.join();
    std::cout << "new sent: " << new_sent << std::endl;
  }

  // wait for all the ops to arrive and then calculate throughput
  int cs = 0;
  while (true) {
    cs = 0;
    for (int i = 0; i < object->num_methods; i++){
      for (int x = 0; x < nr_procs; x++) {
        cs += protocol.repl_object->calls_applied[i][x];
      }
    }
    // std::cout << "received: " << cs << std::endl;
    if (cs == expected_calls)
      break;
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }

  uint64_t global_end = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();

  std::cout << "throughput: "
            << static_cast<double>(num_ops)/static_cast<double>(global_end - local_start) << std::endl;
  object->toString();
  std::this_thread::sleep_for(std::chrono::seconds(60));
  return 0;
}
