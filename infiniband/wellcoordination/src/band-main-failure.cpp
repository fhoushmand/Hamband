#include <cstdlib>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <dory/store.hpp>

#include "band-nb.hpp"

#include "../benchmark/account.hpp"

namespace {
constexpr char FailureKey[] = "hamband-failure-triggered";

struct WorkloadFile {
  int expected_writes = -1;
  std::vector<std::string> calls;
};

std::string workloadDirectory(int replicas, int operations, int percentage,
                              std::string const& usecase) {
  auto const* configured = std::getenv("HAMBAND_WORKLOAD_DIR");
  std::string root = configured == nullptr
                         ? "/scratch/user/u.js213354/Hamband/infiniband/"
                           "wellcoordination/workload"
                         : configured;
  if (!root.empty() && root.back() != '/') {
    root += '/';
  }
  return root + std::to_string(replicas) + "-" +
         std::to_string(operations) + "-" + std::to_string(percentage) +
         "/" + usecase + "/";
}

WorkloadFile readWorkload(std::string const& path) {
  std::ifstream input(path);
  if (!input.is_open()) {
    throw std::runtime_error("Cannot open workload file: " + path);
  }

  WorkloadFile workload;
  std::string line;
  while (std::getline(input, line)) {
    if (line.empty()) {
      continue;
    }
    if (line.front() == '#') {
      if (workload.expected_writes != -1) {
        throw std::runtime_error("Duplicate workload header: " + path);
      }
      workload.expected_writes = std::stoi(line.substr(1));
      continue;
    }
    workload.calls.push_back(line);
  }
  if (workload.expected_writes < 0) {
    throw std::runtime_error("Missing workload header: " + path);
  }
  return workload;
}

std::vector<MethodCall> makeCalls(std::vector<std::string> const& raw,
                                  size_t first, int origin,
                                  int& sequence_number) {
  std::vector<MethodCall> calls;
  calls.reserve(raw.size() - first);
  for (size_t index = first; index < raw.size(); index++) {
    calls.push_back(ReplicatedObject::createCall(
        std::to_string(origin) + "-" + std::to_string(sequence_number++),
        raw[index]));
  }
  return calls;
}

void waitForFailure(dory::MemoryStore& store) {
  std::string value;
  while (!store.get(FailureKey, value)) {
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}
}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 8) {
    throw std::runtime_error(
        "Usage: band-failure <id> <nodes> <operations> <write-percent> "
        "<usecase> <throughput:0|1> <failed-node:1|2>");
  }

  int const id = std::stoi(argv[1]);
  int const replicas = std::stoi(argv[2]);
  int const operations = std::stoi(argv[3]);
  int const percentage = std::stoi(argv[4]);
  std::string const usecase = argv[5];
  bool const calculate_throughput = std::stoi(argv[6]) == 1;
  int const failed_node = std::stoi(argv[7]);
  if (replicas != 4 || id < 1 || id > replicas || operations <= 0 ||
      percentage < 0 || percentage > 100 || usecase != "account" ||
      (failed_node != 1 && failed_node != 2)) {
    throw std::runtime_error("Invalid Account failure experiment arguments");
  }

  std::vector<int> remote_ids;
  for (int node = 1; node <= replicas; node++) {
    if (node != id) {
      remote_ids.push_back(node);
    }
  }

  BankAccount account(BankAccount::InitialBalance);
  account.setID(id)->setNumProcess(replicas)->finalize();
  NB_Wellcoordination protocol(id, remote_ids, &account);
  protocol.rb->hb_active.store(true);
  std::this_thread::sleep_for(std::chrono::seconds(10));

  std::string const directory =
      workloadDirectory(replicas, operations, percentage, usecase);
  WorkloadFile const own =
      readWorkload(directory + std::to_string(id) + ".txt");
  if (own.expected_writes != operations * percentage / 100) {
    throw std::runtime_error("Workload header does not match requested writes");
  }

  int call_sequence = 0;
  std::vector<MethodCall> own_calls = makeCalls(own.calls, 0, id, call_sequence);
  size_t const failure_index = own_calls.size() / 2;
  int const redirector = failed_node == 1 ? 2 : failed_node + 1;
  std::vector<MethodCall> redirected_calls;
  if (id == redirector) {
    WorkloadFile const failed =
        readWorkload(directory + std::to_string(failed_node) + ".txt");
    if (failed.expected_writes != own.expected_writes) {
      throw std::runtime_error("Failed-node workload header differs");
    }
    redirected_calls =
        makeCalls(failed.calls, failed.calls.size() / 2, id, call_sequence);
  }

  auto& store = dory::MemoryStore::getInstance();
  store.set("failure-ready-" + std::to_string(id), "ready");
  for (int node = 1; node <= replicas; node++) {
    std::string value;
    while (!store.get("failure-ready-" + std::to_string(node), value)) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  }

  uint64_t const local_start =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  uint64_t response_sum_ns = 0;
  size_t response_count = 0;
  int own_issued = 0;
  int redirected_issued = 0;
  bool failure_observed = false;

  auto issue = [&](MethodCall const& call, int& counter) {
    auto const start = std::chrono::high_resolution_clock::now();
    auto const response = protocol.request(call, false, false);
    if (response.first != ResponseStatus::NoError) {
      throw std::runtime_error("WRDT request failed during failure experiment");
    }
    if (!calculate_throughput) {
      response_sum_ns +=
          std::chrono::duration_cast<std::chrono::nanoseconds>(response.second -
                                                               start)
              .count();
      response_count++;
    }
    counter++;
  };

  for (size_t index = 0; index < own_calls.size(); index++) {
    if (!failure_observed && index == failure_index) {
      if (id == failed_node) {
        for (size_t group = 0; group < account.synch_groups.size(); group++) {
          protocol.tob[group]->stopHeartbeatThread();
        }
        store.set(FailureKey, std::to_string(failed_node));
        std::cout << "failure injected at node " << id << " after "
                  << own_issued << " local operations" << std::endl;
        std::cout << "issued before failure " << own_issued << " operations"
                  << std::endl;
        std::cout.flush();
        while (true) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      }
      waitForFailure(store);
      failure_observed = true;
      std::cout << "failure observed: node " << failed_node << std::endl;
    }
    issue(own_calls[index], own_issued);
  }

  if (!failure_observed) {
    waitForFailure(store);
    failure_observed = true;
  }

  if (id == redirector) {
    if (failed_node == 1) {
      while (!protocol.tob[0]->amILeader()) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      std::cout << "new leader elected: node " << id << std::endl;
    }
    for (auto const& call : redirected_calls) {
      issue(call, redirected_issued);
    }
  }

  std::cout << "issued " << own_issued << " own operations" << std::endl;
  std::cout << "redirected " << redirected_issued << " operations" << std::endl;
  std::cout << "issued " << own_issued + redirected_issued
            << " total operations" << std::endl;
  protocol.flushOrdered();

  int applied = 0;
  do {
    applied = 0;
    for (int method = 0; method < account.num_methods; method++) {
      for (int origin = 0; origin < replicas; origin++) {
        applied += account.calls_applied[method][origin];
      }
    }
    if (applied < own.expected_writes) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  } while (applied < own.expected_writes);

  uint64_t const global_end =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  double const average_response_us =
      response_count == 0
          ? 0
          : static_cast<double>(response_sum_ns) / 1000.0 /
                static_cast<double>(response_count);
  std::cout << "total average response time for " << response_count
            << " calls: " << average_response_us << std::endl;
  std::cout << "throughput: "
            << static_cast<double>(operations) /
                   static_cast<double>(global_end - local_start)
            << std::endl;
  std::cout << "final state for node " << id << ":" << std::endl;
  account.toString();
  std::cout.flush();

  store.set("finished-failure-" + std::to_string(id), "ready");
  for (int node = 1; node <= replicas; node++) {
    if (node == failed_node) {
      continue;
    }
    std::string value;
    while (!store.get("finished-failure-" + std::to_string(node), value)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  std::cout << "all surviving nodes finished" << std::endl;
  std::cout.flush();
  std::_Exit(0);
}
