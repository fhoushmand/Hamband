#include <stdlib.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include <dory/store.hpp>

#include "band-nb.hpp"

#include "../benchmark/account.hpp"
#include "../benchmark/counter.hpp"
#include "../benchmark/courseware.hpp"
#include "../benchmark/gset.hpp"
#include "../benchmark/orset.hpp"
#include "../benchmark/pnset.hpp"
#include "../benchmark/project.hpp"
#include "../benchmark/register.hpp"
#include "../benchmark/movie.hpp"
#include "../benchmark/rubis.hpp"
#include "../benchmark/shop.hpp"
#include "../benchmark/smallbank.hpp"
#include "../benchmark/twopset.hpp"


int main(int argc, char* argv[]) {
  if (argc < 8) {
    throw std::runtime_error(
        "Usage: band <id> <nodes> <operations> <write-percent> <usecase> "
        "<throughput:0|1> <failed-node:0|id>");
  }
  constexpr int minimum_id = 1;
  int id = std::stoi(argv[1]);
  int nr_procs = static_cast<int>(std::atoi(argv[2]));
  int num_ops = static_cast<int>(std::atoi(argv[3]));
  double write_percentage = static_cast<double>(std::atoi(argv[4]));
  std::string usecase = std::string(argv[5]);
  bool calculate_throughput = (std::atoi(argv[6]) == 1);
  if (nr_procs < 2 || id < minimum_id || id >= minimum_id + nr_procs ||
      num_ops < 0 || write_percentage < 0 || write_percentage > 100) {
    throw std::runtime_error("Invalid process or workload arguments");
  }

  // 0 for no failure
  // 2 for leader failure
  int failed_node = std::atoi(argv[7]);

  std::cout << "number of operations: " << num_ops << std::endl;
  std::cout << "write precentage: "
            << static_cast<double>(write_percentage / 100) << std::endl;
  auto const* configured_workload_dir = std::getenv("HAMBAND_WORKLOAD_DIR");
  std::string loc = configured_workload_dir == nullptr
                        ? "/scratch/user/u.js213354/Hamband/wellcoordination/workload/"
                        : configured_workload_dir;
  if (!loc.empty() && loc.back() != '/') {
    loc += '/';
  }
  loc += std::to_string(nr_procs) + "-" + std::to_string(num_ops) + "-" +
         std::to_string(static_cast<int>(write_percentage));
  loc += "/" + usecase + "/";

  
  std::cout<<"seg0"<<std::endl;
  // Build the list of remote ids
  std::vector<int> remote_ids;
  for (int i = 0, min_id = minimum_id; i < nr_procs; i++, min_id++) {
    if (min_id == id) {
      continue;
    } else {
      remote_ids.push_back(min_id);
    }
  }
  std::cout<<"seg1"<<std::endl;	

  ReplicatedObject* object = nullptr;
  if (usecase == "account") {
    object = new BankAccount(BankAccount::InitialBalance);
  } else if (usecase == "smallbank") {
    object = new SmallBank(100000, 100000000);
  } else if (usecase == "counter") {
    object = new Counter();
  } else if (usecase == "gset") {
    object = new GSet();
  } else if (usecase == "orset") {
    object = new ORSet();
  } else if (usecase == "pnset") {
    object = new PNSet();
  } else if (usecase == "register") {
    object = new Register();
  } else if (usecase == "shop") {
    object = new Shop();
  } else if (usecase == "twopset") {
    object = new TWOPSet();
  } else if (usecase == "movie") {
    object = new Movie();
  } else if (usecase == "rubis") {
    object = new Rubis();
  }else if (usecase == "courseware") {
    object = new Courseware();
    // init object
    for (int i = 0; i < 1000; i++) {
      static_cast<Courseware*>(object)->registerStudent(std::to_string(i));
      static_cast<Courseware*>(object)->addCourse(std::to_string(i));
    }
  }
  else if (usecase == "project") {
    object = new Project();
    // init object
    for (int i = 0; i < 1000; i++) {
      static_cast<Project*>(object)->addEmployee(std::to_string(i));
      static_cast<Project*>(object)->addProject(std::to_string(i));
    }
  }
  if (object == nullptr) {
    throw std::runtime_error("Unknown WRDT use case: " + usecase);
  }
  object->setID(id)->setNumProcess(nr_procs)->finalize();
  
  std::unordered_map<std::string, uint64_t>* response_times =
      new std::unordered_map<std::string, uint64_t>[object->num_methods];
  for (int i = 0; i < object->num_methods; i++)
    response_times[i] = std::unordered_map<std::string, uint64_t>();

  auto& store = dory::MemoryStore::getInstance();
  NB_Wellcoordination protocol(id, remote_ids, object);
  protocol.collect_latency_breakdown =
      (!calculate_throughput && id == 1 && usecase == "account");
  std::this_thread::sleep_for(std::chrono::seconds(10));
  protocol.rb->hb_active.store(true);
  std::cout<<"seg3"<<std::endl;
  int call_id = 0;
  int sent = 0;
  std::string line;
  int expected_calls = 0;
  std::ifstream myfile;
  myfile.open((loc + std::to_string(id) + ".txt").c_str());
  if (!myfile.is_open()) {
    throw std::runtime_error("Cannot open workload file: " + loc +
                             std::to_string(id) + ".txt");
  }
  std::cout<<"seg4"<<std::endl;
  std::vector<MethodCall> requests;
  bool found_expected_calls = false;
  while (getline(myfile, line)) {
    if (line.empty()) {
      continue;
    }
    if (unlikely(line.at(0) == '#')) {
      expected_calls = std::stoi(line.substr(1, line.size()));
      found_expected_calls = true;
      continue;
    }
    std::string sequence_number =
        std::to_string(id) + "-" + std::to_string(call_id++);
    MethodCall call = ReplicatedObject::createCall(sequence_number, line);
    if (call.method_type < 0 || call.method_type >= object->num_methods) {
      throw std::runtime_error("Workload contains an invalid method type");
    }
    requests.push_back(call);
  }
  if (!found_expected_calls) {
    throw std::runtime_error("Workload is missing the expected-write header");
  }

  if(id != 1)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  store.set(std::to_string(id), "ready");
  for(int i = 1; i <= nr_procs; i++)
  {
    std::string value;
    while (!store.get(std::to_string(i), value));
  }
  std::cout << "started sending..." << std::endl;

  uint64_t local_start = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  store.set(std::to_string(id), std::to_string(local_start));


if(calculate_throughput) {
    // start issuing the requests
    for(MethodCall call : requests) {
      std::pair<ResponseStatus, std::chrono::high_resolution_clock::time_point>
          response = protocol.request(call, false, false);
      if (response.first != ResponseStatus::NoError) {
        throw std::runtime_error("WRDT request failed; experiment aborted");
      }
      sent++;
    }
  }
  else {
    // start issuing the requests
    for(MethodCall call : requests) {
      // calculating the response time
      auto start = std::chrono::high_resolution_clock::now();
      std::pair<ResponseStatus, std::chrono::high_resolution_clock::time_point>
          response = protocol.request(call, false, false);
      if (response.first != ResponseStatus::NoError) {
        throw std::runtime_error("WRDT request failed; experiment aborted");
      }
      sent++;
      response_times[call.method_type][call.id] =
          std::chrono::duration_cast<std::chrono::nanoseconds>(response.second -
                                                                    start)
            .count();
    }
  }



  // if(calculate_throughput) {
  //   // start reading from the request file
  //   while (getline(myfile, line)) {
  //   // for(std::string line : requests) {
  //     if (unlikely(line.at(0) == '#')) {
  //       expected_calls = std::stoi(line.substr(1, line.size()));
  //       continue;
  //     }
  //     std::string sequence_number =
  //         std::to_string(id) + "-" + std::to_string(call_id++);
  //     MethodCall call = ReplicatedObject::createCall(sequence_number, line);
  //     std::pair<ResponseStatus, std::chrono::high_resolution_clock::time_point>
  //         response = protocol.request(call, false, false);
  //     if (response.first == ResponseStatus::NoError)
  //       sent++;
  //   }
  // }
  // else {
  //   // start reading from the request file
  //   while (getline(myfile, line)) {
  //     if (unlikely(line.at(0) == '#')) {
  //       expected_calls = std::stoi(line.substr(1, line.size()));
  //       continue;
  //     }

  //     std::string sequence_number =
  //         std::to_string(id) + "-" + std::to_string(call_id++);
  //     MethodCall call = ReplicatedObject::createCall(sequence_number, line);
  //     // calculating the response time
  //     auto start = std::chrono::high_resolution_clock::now();
  //     std::pair<ResponseStatus, std::chrono::high_resolution_clock::time_point>
  //         response = protocol.request(call, false, false);
  //     if (response.first == ResponseStatus::NoError) {
  //       sent++;
  //       response_times[call.method_type][call.id] =
  //           std::chrono::duration_cast<std::chrono::nanoseconds>(response.second -
  //                                                                 start)
  //               .count();
  //     }
  //   }
  // }
  uint64_t local_end = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  // std::cout << "local_throughput:"
  //           << static_cast<double>(num_ops)/static_cast<double>(local_end - local_start) << std::endl;
  std::cout << "issued " << sent << " operations" << std::endl;

  protocol.rb->flush();
  protocol.flushOrdered();

  if(!calculate_throughput){
    double sum = 0;
    double total_sum = 0;
    size_t num = 0;
    for (int i = 0; i < object->num_methods; i++) {
      sum = 0;
      for (auto& pair : response_times[i]){
        sum += static_cast<double>(pair.second);
        total_sum += static_cast<double>(pair.second);
        num++;
      }
      std::cout << "average response time for " << response_times[i].size()
                << " calls to " << i << ": "
                << (response_times[i].empty()
                        ? 0
                        : (sum / 1000) /
                              static_cast<double>(response_times[i].size()))
                << std::endl;
    }
    std::cout << "total average response time for " << num
              << " calls: "
              << (num == 0 ? 0 : (total_sum / 1000) / static_cast<double>(num))
              << std::endl;

    if (id == 1 && usecase == "account") {
      auto& breakdown = protocol.leader_conflict_breakdown;
      double conflict_sum_ns = 0;
      for (auto& pair : response_times[BankAccount::MethodType::WITHDRAW]) {
        conflict_sum_ns += static_cast<double>(pair.second);
      }

      double conflict_avg_us = breakdown.attempted == 0
                                   ? 0
                                   : (conflict_sum_ns / 1000) /
                                         static_cast<double>(breakdown.attempted);
      uint64_t proposed_conflicts = breakdown.no_error + breakdown.dory_error;
      double mu_avg_us = proposed_conflicts == 0
                             ? 0
                             : (static_cast<double>(breakdown.mu_ns) / 1000) /
                                   static_cast<double>(proposed_conflicts);
      double non_mu_avg_us = conflict_avg_us - mu_avg_us;
      double pre_mu_avg_us = proposed_conflicts == 0
                                 ? 0
                                 : (static_cast<double>(breakdown.pre_mu_ns) /
                                    1000) /
                                       static_cast<double>(proposed_conflicts);
      double post_mu_avg_us = proposed_conflicts == 0
                                  ? 0
                                  : (static_cast<double>(breakdown.post_mu_ns) /
                                     1000) /
                                        static_cast<double>(proposed_conflicts);
      double mu_percentage =
          conflict_avg_us > 0 ? (mu_avg_us * 100) / conflict_avg_us : 0;
      double non_mu_percentage =
          conflict_avg_us > 0 ? (non_mu_avg_us * 100) / conflict_avg_us : 0;

      std::cout << "leader conflicting calls attempted: "
                << breakdown.attempted << std::endl;
      std::cout << "leader conflicting calls processed: "
                << breakdown.no_error << std::endl;
      std::cout << "leader conflicting integrity drops: "
                << breakdown.not_permissible << std::endl;
      std::cout << "leader conflicting dory errors: "
                << breakdown.dory_error << std::endl;
      std::cout << "leader conflicting average response time: "
                << conflict_avg_us << std::endl;
      std::cout << "leader conflicting average mu time: " << mu_avg_us
                << std::endl;
      std::cout << "leader conflicting average non-mu time: "
                << non_mu_avg_us << std::endl;
      std::cout << "leader conflicting mu percentage: " << mu_percentage
                << std::endl;
      std::cout << "leader conflicting non-mu percentage: "
                << non_mu_percentage << std::endl;
      std::cout << "leader conflicting average pre-mu time: "
                << pre_mu_avg_us << std::endl;
      std::cout << "leader conflicting average post-mu time: "
                << post_mu_avg_us << std::endl;
    }
  }

  // wait for all the ops to arrive and then calculate throughput
  int cs = 0;
  int sz = static_cast<int>(object->synch_groups.size());
  while (true) {
    cs = 0;
    for (int i = 0; i < object->num_methods; i++)
      for (int x = 0; x < nr_procs; x++) cs += protocol.repl_object->calls_applied[i][x];
    // std::cout << "received: " << cs << std::endl;
    if (failed_node == 0) {
      if (cs >= expected_calls) {
        break;
      }
    } else if (sz == 1) {
      if (cs >= ((id != 1) ? (expected_calls - 1) : expected_calls)) {
        break;
      }
    } else if (cs >= ((id > sz) ? (expected_calls - sz)
                                 : (expected_calls - 1))) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  uint64_t global_end = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();

  std::cout << "throughput: "
            << static_cast<double>(num_ops)/static_cast<double>(global_end - local_start) << std::endl;

  std::cout << "final state for node " << id << ":" << std::endl;
  object->toString();
  std::cout.flush();

  store.set("finished-" + std::to_string(id), "ready");
  for (int i = 1; i <= nr_procs; i++) {
    std::string value;
    while (!store.get("finished-" + std::to_string(i), value)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  std::cout << "all nodes finished" << std::endl;
  std::cout.flush();
  std::_Exit(0);
}
