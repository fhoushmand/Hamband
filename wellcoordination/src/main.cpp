#include <stdlib.h>

#include <dory/store.hpp>

// #include "nb-protocol-courseware.hpp"
#include "mu-courseware.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Provide the id of the process as argument");
  }
  size_t num_ops = static_cast<size_t>(std::atoi(argv[2]));
  std::cout << "number of operations " << num_ops << std::endl;
  constexpr int nr_procs = 4;
  constexpr int minimum_id = 1;
  int id = 0;
  switch (argv[1][0]) {
    case '1':
      id = 1;
      break;
    case '2':
      id = 2;
      break;
    case '3':
      id = 3;
      break;
    case '4':
      id = 4;
      break;
    case '5':
      id = 5;
      break;
    default:
      throw std::runtime_error("Invalid id");
  }

  // Build the list of remote ids
  std::vector<int> remote_ids;
  for (int i = 0, min_id = minimum_id; i < nr_procs; i++, min_id++) {
    if (min_id == id) {
      continue;
    } else {
      remote_ids.push_back(min_id);
    }
  }
  auto& store = dory::MemoryStore::getInstance();
  // NB_Wellcoordination protocol(id, remote_ids);
  Mu protocol(id, remote_ids);
  store.set(std::to_string(id), "1");
  int ready;
  // do{
  //   ready = 0;
  //   for(size_t i = id; i <= nr_procs; i++)
  //   {
  //     std::string val;
  //     store.get(std::to_string(i), val);
  //     if(val.length() != 0){
  //       // std::cout << "val: " << val << std::endl;
  //       ready += std::stoi(val);
  //     }
  //   }
  // }while(ready != nr_procs - id + 1);
  std::cout << "started at: "
            << std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch())
                   .count()
            << std::endl;

  // std::this_thread::sleep_for(std::chrono::seconds(5));
  int num_withdrawls = 0;
  for (size_t i = 1; i <= num_ops; i++)
    if (i % 23 == 0 && i % 17 != 0) num_withdrawls++;

  uint64_t throughput_end;
  uint64_t throughput_start =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  size_t i = 0;
  uint64_t start;

  std::this_thread::sleep_for(std::chrono::seconds(8 - id));
  for (i = 1; i <= num_ops; i++) {
    std::string callStr;
    // addCourse
    if (id == 1 && i % 199 == 0) {
      callStr = "0 " + std::to_string(std::rand() % 100);
    }
    // deleteCourse
    else if (id == 1 && i % 101 == 0) {
      callStr = "1 " + std::to_string(std::rand() % 100);
    }
    // enroll
    else if (id == 1 && i % 307 == 0)
      callStr = "2 " + std::to_string(std::rand() % 500) + "-" +
                std::to_string(std::rand() % 100);
    // registerStudent
    else if (id == 1 && i % 31 == 0)
      callStr = "3 " + std::to_string(std::rand() % 500);
    // query
    else
      callStr = "4";
    // std::cout << callStr << std::endl;
    protocol.request(callStr, false, false);
    // std::this_thread::sleep_for(std::chrono::microseconds(500));
  }

  // while(true)
  // {
  //   if(protocol.withdraws_applied == ((id != 1) ? num_withdrawls - 1 :
  //   num_withdrawls))
  //     break;
  //   std::this_thread::sleep_for(std::chrono::seconds(2));
  // }
  // std::this_thread::sleep_for(std::chrono::seconds(2));
  throughput_end =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  std::cout << "trhoughput_time: " << throughput_end - throughput_start
            << std::endl;

  for (int i = 0; i < protocol.courseware.num_methods; i++) {
    double sum = 0;
    for (auto& pair : protocol.response_times[i])
      sum += static_cast<double>(pair.second);
    std::cout << "average response time for " << protocol.response_times[i].size() << " calls to " << i << ": "
              << sum / static_cast<int>(protocol.response_times[i].size())
              << std::endl;
  }

  // uint64_t sum = 0;
  // std::cout << "total average response time: "
  //           << static_cast<double>(sum) /
  //                  static_cast<int>(protocol.response_time.size())
  //           << std::endl;

  std::thread reader([&] {
    std::string line;
    while (std::getline(std::cin, line)) {
      protocol.request(line, true, false);
    }
  });
  reader.join();

  return 0;
}
