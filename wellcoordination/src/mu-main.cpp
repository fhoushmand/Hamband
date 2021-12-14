#include <stdlib.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include <dory/store.hpp>

#include "mu.hpp"

#include "courseware.hpp"
#include "bank_account.hpp"
#include "counter.hpp"
#include "gset.hpp"
#include "movie.hpp"
#include "register.hpp"
#include "cart.hpp"
#include "shop.hpp"

MethodCall createCall(std::string id, std::string call);

MethodCall createCall(std::string id, std::string call) {
  int method_type;
  size_t spaceIndex = call.find_first_of(' ');
  if (spaceIndex == std::string::npos)
    method_type = std::stoi(call);
  else
    method_type = std::stoi(call.substr(0, spaceIndex));

  std::string arg = call.substr(spaceIndex + 1, call.size());

  return MethodCall(id, method_type, arg);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Provide the id of the process as argument");
  }
  int nr_procs = static_cast<int>(std::atoi(argv[2]));
  int num_ops = static_cast<int>(std::atoi(argv[3]));
  double write_percentage = static_cast<double>(std::atoi(argv[4]));
  std::string usecase = std::string(argv[5]);
  bool calculate_throughput = (std::atoi(argv[6]) == 1);
  
  double total_writes = num_ops * (write_percentage / 100);
  bool leader_failure_experiment = false;
  bool follower_failure_experiment = false;
  std::cout << "number of operations: " << num_ops << std::endl;
  std::cout << "write precentage: "
            << static_cast<double>(write_percentage / 100) << std::endl;
  std::string loc =
      "/rhome/fhous001/farzin/FastChain/dory/wellcoordination/workload/";
  loc += std::to_string(nr_procs) + "-" + std::to_string(num_ops) + "-" +
         std::to_string(static_cast<int>(write_percentage));
  loc += "/" + usecase + "/";

  constexpr int minimum_id = 1;
  std::string idstr(argv[1], argv[1] + 1);
  int id = std::stoi(idstr);

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
  if(usecase == "counter")
  {
    object = new Counter();
  }
  else if(usecase == "register")
  {
    object = new Register();
  }
  else if(usecase == "cart")
  {
    object = new Cart();
  }
  else if(usecase == "shop")
  {
    object = new Shop();
  }
  else if(usecase == "gset")
  {
    object = new GSet();
  }
  else if(usecase == "courseware")
  {
    object = new Courseware();
    // init object
    for (int i = 0; i < 1000; i++) {
      static_cast<Courseware*>(object)->registerStudent(std::to_string(i));
      static_cast<Courseware*>(object)->addCourse(std::to_string(i));
    }
  }
  else if(usecase == "account")
  {
    object = new BankAccount(100000);
  }
  else if(usecase == "movie")
  {
    object = new Movie();
  }
  // no need to track deps while using mu
  object->dependency_relation.clear();

  MethodCallFactory callFactory = MethodCallFactory(object, nr_procs);
  std::unordered_map<std::string, uint64_t>* response_times =
      new std::unordered_map<std::string, uint64_t>[object->num_methods];
  for (int i = 0; i < object->num_methods; i++)
    response_times[i] = std::unordered_map<std::string, uint64_t>();


  std::atomic<bool> own_requests_done = false;
  auto& store = dory::MemoryStore::getInstance();
  Mu protocol(id, remote_ids, object);
  std::this_thread::sleep_for(std::chrono::seconds(10));


  int call_id = 0;
  std::ifstream leader_requests;
  std::ifstream follower_requests;
  std::string l;
  int new_sent = 0;
  std::thread leaderChange;
  if (leader_failure_experiment) {
    leader_requests.open((loc + "leader" + ".txt").c_str());
    leaderChange = std::thread([&] {
      while (true) {
        if (own_requests_done.load() &&
        protocol.synchSenders->amILeader()) {
          std::cout << "started sending remaining leader messages" << std::endl; 
          while (getline(leader_requests, l)) {
            std::string sequence_number =
            std::to_string(id) + "-" + std::to_string(call_id++);
            MethodCall call = createCall(sequence_number, l);
            if (protocol.request(call, false, false).first == ResponseStatus::NoError) new_sent++;
          }
          std::cout << "finished sending" << std::endl;
          break;
        }
      }
      return;
    });
  }
  
  std::thread followerChange;
  if (follower_failure_experiment) {
    follower_requests.open((loc + "follower" + ".txt").c_str());
    followerChange = std::thread([&] {
      while (true) {
        if (own_requests_done.load()) {
          std::cout << "started sending remaining follower messages" << std::endl;
          while (getline(follower_requests, l)) {
            std::string sequence_number =
            std::to_string(id) + "-" + std::to_string(call_id++);
            MethodCall call = createCall(sequence_number, l);
            if (protocol.request(call, false, false).first == ResponseStatus::NoError) new_sent++;
          }
          std::cout << "finished sending" << std::endl;
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

  std::cout << "started sending..." << std::endl;

  if(id != 1)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  store.set(std::to_string(id), "ready");
  for(int i = 1; i < nr_procs; i++)
  {
    std::string value;
    while (!store.get(std::to_string(i), value));
  }
  uint64_t local_start = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  store.set(std::to_string(id), std::to_string(local_start));

  for (int i = 1; i <= nr_procs; i++) {
    if (id != 1 && id != i) continue;
    std::string line;
    std::ifstream myfile;
    std::cout << "reading from file " << i << std::endl;
    myfile.open((loc + std::to_string(i) + ".txt").c_str());
    // start reading from the request file
    while (getline(myfile, line)) {
      if (unlikely(line.at(0) == '#')) {
        expected_calls = std::stoi(line.substr(1, line.size()));
        continue;
      }
      // failure
      if (unlikely(line.at(0) == 'X')) {
        std::cout << "stoping heartbeat thread and waiting..." << std::endl;
        protocol.synchSenders->stopHeartbeatThread();
        break;
      }
      
      std::string sequence_number =
          std::to_string(id) + "-" + std::to_string(call_id++);
      MethodCall call = createCall(sequence_number, line);      
      // don't do reads of others if you are the leader
      if(id == 1 && i != 1 && call.method_type == object->read_method) continue;

      // calculating the response time
      if(!calculate_throughput){
        auto start = std::chrono::high_resolution_clock::now();
        std::pair<ResponseStatus, std::chrono::high_resolution_clock::time_point>
            response = protocol.request(call, false, false);
        if (response.first == ResponseStatus::NoError) {
          sent++;
          response_times[call.method_type][call.id] =
              std::chrono::duration_cast<std::chrono::nanoseconds>(response.second -
                                                                  start)
                  .count();
        }
      }
      else{
        std::pair<ResponseStatus, std::chrono::high_resolution_clock::time_point>
            response = protocol.request(call, false, false);
        if (response.first == ResponseStatus::NoError)
          sent++;
      }
    }
  }
  uint64_t local_end = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  std::cout << "issued " << sent << " operations" << std::endl;

  // wait for the new leader to finish remaining operations
  if (follower_failure_experiment) {
    if (id == 2) {
      own_requests_done.store(true);
      followerChange.join();
      std::cout << "new sent: " << new_sent << std::endl;
    }
  }
  if (leader_failure_experiment) {
    if (id == 2) {
      own_requests_done.store(true);
      leaderChange.join();
      std::cout << "new sent: " << new_sent << std::endl;
    }
  }

//   std::cout << "local_throughput_time: "
//             << local_end - local_start << std::endl;

	if(id == 1){
  	std::cout << "throughput: "
    	        << static_cast<double>(num_ops)/static_cast<double>(local_end - local_start) << std::endl;
  	object->toString();
	}

  double sum = 0;
  double total_sum = 0;
  size_t num = 0;
  for (int i = 0; i < object->num_methods; i++) {
    total_sum += sum;
    sum = 0;
    for (auto& pair : response_times[i])
      sum += static_cast<double>(pair.second);
		num += response_times[i].size();
    std::cout << "average response time for " << response_times[i].size()
              << " calls to " << i << ": "
              << (sum/1000) / static_cast<int>(response_times[i].size()) << std::endl;
  }
  std::cout << "total average response time for " << num
            << " calls: " << (total_sum/1000) / static_cast<int>(num) << std::endl;
	
  std::this_thread::sleep_for(std::chrono::seconds(60));
  return 0;
}