#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "gset.hpp"

int main(int argc, char* argv[]) {
  std::string loc =
      "/rhome/fhous001/farzin/FastChain/dory/wellcoordination/workload/";

  int nr_procs = static_cast<int>(std::atoi(argv[1]));
  int num_ops = static_cast<int>(std::atoi(argv[2]));
  double write_percentage = static_cast<double>(std::atoi(argv[3]));

  loc += std::to_string(nr_procs) + "-" + std::to_string(num_ops) + "-" +
         std::to_string(static_cast<int>(write_percentage));
  loc += "/gset/";

  std::ofstream* outfile = new std::ofstream[nr_procs];
  std::vector<std::string>* calls = new std::vector<std::string>[nr_procs];
  for (int i = 0; i < nr_procs; i++) {
    remove((loc + std::to_string(i + 1) + ".txt").c_str());
    outfile[i].open(loc + std::to_string(i + 1) + ".txt", std::ios_base::app);
    calls[i] = std::vector<std::string>();
  }

  GSet* test = new GSet();
  MethodCallFactory factory = MethodCallFactory(test, nr_procs);

  write_percentage /= 100;
  int num_replicas = nr_procs;
  double total_writes = num_ops * write_percentage;
  int queries = num_ops - total_writes;

  int num_conflicting_write_methods = 1;
  int num_nonconflicting_write_methods = 0;
  int num_read_methods = 1;

  std::cout << "ops: " << num_ops << std::endl;
  std::cout << "write_perc: " << write_percentage << std::endl;
  std::cout << "writes: " << total_writes << std::endl;
  std::cout << "reads: " << queries << std::endl;

  int expected_calls_per_update_method =
      total_writes /
      (num_conflicting_write_methods + num_nonconflicting_write_methods);
  std::cout << "expected #calls per update method "
            << expected_calls_per_update_method << std::endl;

  std::cout << "expected #calls per conflicting writes in leader "
            << expected_calls_per_update_method << std::endl;
  std::cout << "total conflicting #calls in leader "
            << expected_calls_per_update_method * num_conflicting_write_methods
            << std::endl;

  int expected_nonconflicting_write_calls_per_follower =
      (total_writes -
       expected_calls_per_update_method * num_conflicting_write_methods) /
      (nr_procs - 1);

  std::cout << "expected #calls per nonconflicting writes in follower "
            << expected_nonconflicting_write_calls_per_follower << std::endl;
  std::cout << "total nonconflicting #calls in followers "
            << expected_nonconflicting_write_calls_per_follower *
                   num_nonconflicting_write_methods * (nr_procs - 1)
            << std::endl;

  int write_calls =
      expected_calls_per_update_method * num_conflicting_write_methods +
      expected_nonconflicting_write_calls_per_follower *
          num_nonconflicting_write_methods * (nr_procs - 1);

  // first allocating writes operations to the nodes
  for (int i = 1; i <= num_replicas; i++) {
    // leader
    if (i == 1) {
      // conflicting calls are sent to the leader (first process) -- later this
      // will change to handeling multiple leaders
      for (int type = 0; type <= 0; type++) {
        int count = 0;
        for (; count < expected_calls_per_update_method;) {
          std::string callStr;
          if (type == 0) {
            std::string c_id = std::to_string(std::rand() % 5);
            callStr = "0 " + c_id;
          }

          MethodCall call = factory.createCall("id", callStr);
          if (test->isPermissible(call)) {
            test->execute(call);
            calls[i - 1].push_back(callStr);
            count++;
          }
        }
      }
    }
    // follower
    else {
      // non-conflicting write method
      for (int count = 0;
           count < expected_nonconflicting_write_calls_per_follower; count++) {
        std::string callStr;
        // registerStudent
        std::string s_id = std::to_string(std::rand() % 1000);
        callStr = "3 " + s_id;

        MethodCall call = factory.createCall("id", callStr);
        test->execute(call);
        calls[i - 1].push_back(callStr);
      }
      // for(int i = 0; i < queries/(nr_procs-1); i++)
      //   calls.push_back(std::string("4"));
    }
  }

  // allocate reads to the nodes
  int q = num_ops - write_calls;
  std::cout << "q: " << q << std::endl;

  // if(calls[0].size() > calls[1].size())
  int read_calls = q;
  int index = 0;

  std::cout << "after adding writes nodes" << std::endl;
  for (int i = 0; i < nr_procs; i++)
    std::cout << i + 1 << " size: " << calls[i].size() << std::endl;

  while (calls[0].size() > calls[1].size() && read_calls != 0) {
    calls[(index % (nr_procs - 1)) + 1].push_back(std::string("1"));
    read_calls--;
    index++;
  }
  std::cout << "after adding reads to followers" << std::endl;
  for (int i = 0; i < nr_procs; i++)
    std::cout << i + 1 << " size: " << calls[i].size() << std::endl;

  if (read_calls != 0) {
    for (int i = 0; i < nr_procs; i++)
      for (int j = 0; j < read_calls / nr_procs; j++)
        calls[i].push_back(std::string("1"));
    std::cout << "after adding reads to all" << std::endl;
    for (int i = 0; i < nr_procs; i++)
      std::cout << i + 1 << " size: " << calls[i].size() << std::endl;
  }

  for (int i = 0; i < nr_procs; i++) {
    calls[i].insert(calls[i].begin(),
                    std::string("#" + std::to_string(write_calls)));
    std::random_shuffle(calls[i].begin() + 1, calls[i].end());
  }

  //   // outfile.open(loc + std::to_string(i) + ".txt", std::ios_base::app);
  for (int i = 0; i < nr_procs; i++) {
    for (int x = 0; x < calls[i].size(); x++)
      outfile[i] << calls[i][x] << std::endl;
    outfile[i].close();
  }

  std::cout << "expected calls to receive: " << write_calls << std::endl;
  return 0;
}
