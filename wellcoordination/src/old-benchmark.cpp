#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>

#include "courseware.hpp"

int main(int argc, char* argv[]) {

  std::string loc = "/rhome/fhous001/farzin/FastChain/dory/wellcoordination/workload/";

  int nr_procs = static_cast<int>(std::atoi(argv[1]));
  int num_ops = static_cast<int>(std::atoi(argv[2]));
  double write_percentage = static_cast<double>(std::atoi(argv[3]));

  loc += std::to_string(nr_procs) + "-" + std::to_string(num_ops) + "-" + std::to_string(static_cast<int>(write_percentage));
  loc += "/courseware/";

  std::ofstream outfile;

  Courseware* test = new Courseware();
  for(int i = 0; i < 1000; i++)
    {
        test->registerStudent(std::to_string(i));
        test->addCourse(std::to_string(i));
    }
  MethodCallFactory factory = MethodCallFactory(test, nr_procs);

  write_percentage /= 100;
  int num_replicas = nr_procs;
  double total_writes = num_ops * write_percentage;
  int queries = num_ops - total_writes;

  std::cout << "ops: " << num_ops << std::endl;
  std::cout << "write_perc: " << write_percentage << std::endl;
  std::cout << "writes: " << total_writes << std::endl;
  std::cout << "reads: " << queries << std::endl;

  int num_update_calls_per_replica = 0;
  int expected_write_calls = 0;
  
  for (int i = 1; i <= num_replicas; i++) {
    remove((loc + std::to_string(i) + ".txt").c_str());
    std::vector<std::string> calls;
    // leader
    if (i == 1) {
      // conflicting calls are sent to the leader (first process) -- later this will
      // change to handeling multiple leaders
      for (int type = 0; type <= 3; type++) {
        if (type == 3)
          num_update_calls_per_replica =
              total_writes / (test->update_methods.size() * num_replicas);
        else
          num_update_calls_per_replica =
              total_writes / test->update_methods.size();
        for (int count = 0; count < num_update_calls_per_replica;) {
          std::string callStr;
          if (type == 0) {
            std::string c_id = std::to_string(1001 + std::rand() % 100);
            callStr = "0 " + c_id;
          }
          // deleteCourse
          else if (type == 1) {
            std::string c_id = std::to_string(1001 + std::rand() % 100);
            callStr = "1 " + c_id;
          }
          // enroll
          else if (type == 2) {
            std::string s_id = std::to_string(std::rand() % 1000);
            std::string c_id = std::to_string(std::rand() % 1000);
            callStr = "2 " + s_id + "-" + c_id;
          }
          else if(type == 3)
          {
            std::string s_id = std::to_string(1001 + std::rand() % 1000);
            callStr = "3 " + s_id;
          }

          MethodCall call = factory.createCall("id", callStr);
          if (test->isPermissible(call)) {
            test->execute(call);
            calls.push_back(callStr);
            count++;
          }
        }
      }
    }
    // follower
    else {
      num_update_calls_per_replica =
          total_writes / (test->update_methods.size() * num_replicas);
      for (int count = 0; count < num_update_calls_per_replica; count++) {
        std::string callStr;
        // registerStudent
        std::string s_id = std::to_string(1000 + std::rand() % 1000);
        callStr = "3 " + s_id;

        MethodCall call = factory.createCall("id", callStr);
        if (test->isPermissible(call)) {
          test->execute(call);
          calls.push_back(callStr);
        }
      }
    }
    expected_write_calls += calls.size();
    while(calls.size() < num_ops/num_replicas)
        calls.push_back(std::string("4"));

    std::random_shuffle(calls.begin(), calls.end());
    outfile.open(loc + std::to_string(i) + ".txt", std::ios_base::app);
    for (auto& str : calls) outfile << str << std::endl;
    outfile.close();
  }
  std::cout << "expected_calls_received: " << expected_write_calls << std::endl;
  return 0;
}
