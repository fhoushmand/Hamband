#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "twopset.hpp"

int main(int argc, char* argv[]) {
  std::string loc =
      "/users/jsaber/binHamband/workload/";

  int nr_procs = static_cast<int>(std::atoi(argv[1]));
  int num_ops = static_cast<int>(std::atoi(argv[2]));
  double write_percentage = static_cast<double>(std::atoi(argv[3]));

  loc += std::to_string(nr_procs) + "-" + std::to_string(num_ops) + "-" +
         std::to_string(static_cast<int>(write_percentage));
  loc += "/twopset/";

  std::ofstream* outfile = new std::ofstream[nr_procs];
  std::vector<std::string>* calls = new std::vector<std::string>[nr_procs];
  for (int i = 0; i < nr_procs; i++) {
    remove((loc + std::to_string(i + 1) + ".txt").c_str());
    outfile[i].open(loc + std::to_string(i + 1) + ".txt", std::ios_base::app);
    calls[i] = std::vector<std::string>();
  }
  TWOPSet* test = new TWOPSet();
  
  write_percentage /= 100;
  int num_replicas = nr_procs;
  double total_writes = num_ops * write_percentage;
  int queries = num_ops - total_writes;

  int num_nonconflicting_write_methods = 2;
  int num_read_methods = 1;

  std::cout << "ops: " << num_ops << std::endl;
  std::cout << "write_perc: " << write_percentage << std::endl;
  std::cout << "writes: " << total_writes << std::endl;
  std::cout << "reads: " << queries << std::endl;

  int expected_calls_per_update_method = total_writes /num_nonconflicting_write_methods;
  std::cout << "expected #calls per update method "
            << expected_calls_per_update_method << std::endl;

  int expected_calls_per_update_method_per_node = expected_calls_per_update_method / nr_procs;

  std::cout << "expected #calls per nonconflicting writes in nodes "
            << expected_calls_per_update_method_per_node << std::endl;

  int write_calls =
      expected_calls_per_update_method * num_nonconflicting_write_methods;

  int write_calls_issued = 0;
  int reads_issues = 0;
      
  // first allocating writes operations to the nodes
  for (int i = 1; i <= num_replicas; i++) {
    // non-conflicting write method
      for (int count = 0;
          count < expected_calls_per_update_method_per_node; count++) {
          std::string element = std::to_string(std::rand() % 1000000);
          std::string callStr;
          callStr = "0 " + element;
          calls[i - 1].push_back(callStr);
          for(int x = 0; x < static_cast<int>(1/write_percentage)+15; x++)
          {
            if(calls[i - 1].size() == num_ops/nr_procs) 
              break;
            calls[i - 1].push_back(std::string("2"));
            reads_issues++;
          }
          callStr = "1 " + element;
          calls[i - 1].push_back(callStr);
          write_calls_issued += 2;
      }
      
  }
  

  std::cout << "write calls issued: " << write_calls_issued << std::endl;
  std::cout << "read calls issued: " << reads_issues << std::endl;

  // allocate reads to the nodes
  int q = num_ops - write_calls_issued;  
  int read_calls = q - reads_issues;
  std::cout << "extra read calls needed: " << read_calls << std::endl;
  
  int index = 0;

  std::cout << "after adding writes to nodes" << std::endl;
  for (int i = 0; i < nr_procs; i++)
    std::cout << i + 1 << " size: " << calls[i].size() << std::endl;


  if (read_calls != 0) {
    for (int i = 0; i < nr_procs; i++){
      for (int j = 0; j < read_calls / nr_procs; j++)
      {
        calls[i].push_back(std::string("2"));
      }
    }
  }


    std::cout << "after adding reads to all" << std::endl;
    for (int i = 0; i < nr_procs; i++)
      std::cout << i + 1 << " size: " << calls[i].size() << std::endl;


  for (int i = 0; i < nr_procs; i++) {
    calls[i].insert(calls[i].begin(),
                    std::string("#" + std::to_string(write_calls_issued)));
    // std::random_shuffle(calls[i].begin() + 1, calls[i].end());
  }

  for (int i = 0; i < nr_procs; i++) {
    for (int x = 0; x < calls[i].size(); x++)
      outfile[i] << calls[i][x] << std::endl;
    outfile[i].close();
  }

  std::cout << "expected calls to receive: " << write_calls_issued << std::endl;
  return 0;
}
