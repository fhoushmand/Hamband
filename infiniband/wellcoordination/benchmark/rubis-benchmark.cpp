#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "rubis.hpp"
//------------opcode------- number of parameters 

//sellItem    0   2(id+value)
//storeBuyNow 1   2(id+value) by leader 
//registerUser 2   1(id)       by leader
//openAuction  3  1(id)        by default consider 100 auctions are open. 
//placeBid 4      3(auctionid+userid+value)
//placeBid 5      
//query 6  like movie does not consider read. 

int main(int argc, char* argv[]) {
  std::string loc =
      "/users/jsaber/binHamband/workload/";

  int nr_procs = static_cast<int>(std::atoi(argv[1]));
  int num_ops = static_cast<int>(std::atoi(argv[2]));
  double write_percentage = static_cast<double>(std::atoi(argv[3]));

  loc += std::to_string(nr_procs) + "-" + std::to_string(num_ops) + "-" +
         std::to_string(static_cast<int>(write_percentage));
  loc += "/rubis/";

  std::ofstream* outfile = new std::ofstream[nr_procs];
  std::vector<std::string>* calls = new std::vector<std::string>[nr_procs];
  for (int i = 0; i < nr_procs; i++) {
    remove((loc + std::to_string(i + 1) + ".txt").c_str());
    outfile[i].open(loc + std::to_string(i + 1) + ".txt", std::ios_base::app);
    calls[i] = std::vector<std::string>();
  }

  Rubis* test = new Rubis();

  // MethodCallFactory factory = MethodCallFactory(test, nr_procs);

  write_percentage /= 100;
  int num_replicas = nr_procs;
  double total_writes = num_ops * write_percentage;
  int queries = num_ops - total_writes;

  int num_conflicting_write_methods = 4;
  int num_nonconflicting_write_methods = 2;
  int num_read_methods = 0;

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

  std::cout << "expected #calls per nonconflicting writes in followers "
            << expected_nonconflicting_write_calls_per_follower << std::endl;
  std::cout << "total nonconflicting #calls in followers "
            << expected_nonconflicting_write_calls_per_follower *
                   num_nonconflicting_write_methods * (nr_procs - 1)
            << std::endl;

  int write_calls =
      expected_calls_per_update_method * num_conflicting_write_methods +
      expected_nonconflicting_write_calls_per_follower *
          num_nonconflicting_write_methods * (nr_procs - 1);



  std::vector<std::string>* open = new std::vector<std::string>[nr_procs];

  // first allocating writes operations to the nodes
  for (int i = 1; i <= num_replicas; i++) {
    // first leader
    if (i == 1) {
      for (int type = 0; type <= 1; type++) {
        int count = 0;
        for (; count < expected_calls_per_update_method;) {
          // storeBuyNow
          // Buy items with id 0 - 99. 0-5
          std::string callStr;
          if (type == 0) {
            std::string i_id = std::to_string(std::rand() % 100);
            std::string value = std::to_string(std::rand() % 5);
            callStr = "1 " + i_id + "-" + value;
          }
          if (type == 1) {
            std::string u_id = std::to_string(100 + std::rand() % 100);
            callStr = "2 " + u_id;
          }
          MethodCall call = ReplicatedObject::createCall("id", callStr);
          if (test->isPermissible(call)) {
            test->execute(call);
            calls[i - 1].push_back(callStr);
            count++;
          }
        }
      }
    }

    // second leader
    else if (i == 2) {
      for (int type = 0; type <= 1; type++) {
        int count = 0;
        for (; count < expected_calls_per_update_method;) {
          std::string callStr;
          if (type == 0) {
            // placeBid
            // Bid with user id 0 - 99 and item id 0 - 99
            std::string a_id = std::to_string(std::rand() % 100); //auction id
            std::string u_id = std::to_string(std::rand() % 100); //user id
            std::string value = std::to_string(std::rand() % 1000); //bid value

            callStr = "3 " + a_id + "-" + u_id + "-" + value;
          }
          if (type == 1) {
            //Close Auctions
            std::string i_id = std::to_string(std::rand() % 100); //auction id
            //std::string stock = std::to_string(1 + std::rand() % 1000); //stock

            //open[i - 1].push_back(i_id); 
            callStr = "5 " + i_id;
          }
          MethodCall call = ReplicatedObject::createCall("id", callStr);
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
      for (int type = 0; type <= 1; type++) {
        for (int count = 0; count <
        expected_nonconflicting_write_calls_per_follower; count++) {
          std::string callStr;
          if (type == 0) {
            // sellitem 
            std::string s_id = std::to_string(100 + std::rand() % 100);
            std::string value = std::to_string(std::rand() % 1000);
            callStr = "0 " + s_id+ "-"+ value;
          }
          if (type == 1) {
            //Open Auctions
            // Open 100 - 199, with different stock values
            // Save the opened ids so we can close them after shuffling. 
            std::string i_id = std::to_string(100 + std::rand() % 100); //auction id
            std::string stock = std::to_string(1 + std::rand() % 1000); //stock

            open[i - 1].push_back(i_id); 
            callStr = "4 " + i_id + "-" + stock;
          }


          MethodCall call = ReplicatedObject::createCall("id", callStr);
          test->execute(call);
          calls[i - 1].push_back(callStr);
        }
      }
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

  for (int i = 0; i < nr_procs; i++) {
    calls[i].insert(calls[i].begin(),
                    std::string("#" + std::to_string(write_calls)));
    std::random_shuffle(calls[i].begin() + 1, calls[i].end());
  }

  for (int i = 0; i < nr_procs; i++) {
    for (int x = 0; x < calls[i].size(); x++)
      outfile[i] << calls[i][x] << std::endl;
    outfile[i].close();
  }

  std::cout << "expected calls to receive: " << write_calls << std::endl;
  return 0;
}
