#include "nb-protocol.hpp"

using namespace hamsaz;

// void read(BE_Broadcast& sender);

// void read(BE_Broadcast& sender) {
//   // reading (remote and local)
//   // while (true) {
//   std::string line;
//   while (std::getline(std::cin, line)) {
//     int i = std::stoi(line);
//     // ParsedCall pslot;
//     // if (sender.partIter.hasNext(i)) {
//       // pslot = ParsedCall(sender.partIter.next(i).location(i));
//       do {
//         // auto has_next = sender.partIter.sampleNext(i);
//         // if (!has_next) {
//         // continue;
//         // }

//         // if (has_next) {
//         // while (sender.commit_iter[i].hasNext(100000)) {
//         // sender.commit_iter[i].next();
//         // ParsedCall pslot(sender.commit_iter[i].location());
//         ParsedCall pslot = ParsedCall(sender.partIter.location(i));
//         auto [buf, len] = pslot.payload();
//         if (!pslot.isPopulated()) break;
//         std::cout << "buffer: " << buf << std::endl;
//         if (pslot.hasDeps()) {
//           printf("deps: %d, %d, %d", pslot.dependencies()[0],
//                  pslot.dependencies()[1], pslot.dependencies()[2]);
//           std::cout << std::endl;
//           // }
//         }
//         sender.partIter.next(i);
//         // pslot = ParsedCall(sender.partIter.location(i));
//         // }
//       } while (sender.partIter.hasNext(i));
//     // }
//     sender.partIter.reset(i);
//   }

//   // }
// }

int main(int argc, char* argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Provide the id of the process as argument");
  }
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
  hamsaz::NB_Wellcoordination protocol(id, remote_ids);
  std::this_thread::sleep_for(std::chrono::seconds(8));

  size_t i = 0;
  for (i = 0; i < 100000; i++) {
    std::string callType = (i % 10 == 0) ? "deposit" : "query";
    std::string argument = (i % 10 == 0) ? "10" : "";
    std::string seq = std::to_string(id) + "-" + std::to_string(i) + "-" + callType;

    BankAccountCall q1 = BankAccountCall(seq, callType, argument);
    protocol.request(q1, false, false);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  std::unordered_map<std::string,std::pair<uint64_t,size_t>> response_time_per_method;
  response_time_per_method.emplace("deposit", std::make_pair(0,0));
  response_time_per_method.emplace("withdraw", std::make_pair(0,0));
  response_time_per_method.emplace("query", std::make_pair(0,0));

  uint64_t sum = 0;
  for (auto& res : protocol.response_time) 
  {
    sum += res.second;
    size_t index = res.first.find_last_of('-');
    std::string method = res.first.substr(index + 1, res.first.length());
    response_time_per_method[method].first += res.second;
    response_time_per_method[method].second++;
  }

  for(auto &m : response_time_per_method)
  {
    std::cout << m.first << " average response time: "
            << static_cast<double>(m.second.first) /
                   static_cast<int>(m.second.second)
            << std::endl;
  
  }

  std::cout << "total average response time: "
            << static_cast<double>(sum) /
                   static_cast<int>(protocol.response_time.size())
            << std::endl;
  

  // BankAccountCall d1 = BankAccountCall("deposit", "10");
  // protocol.request(d1);
  // }
  //   std::this_thread::sleep_for(std::chrono::seconds(1));

  //   protocol.request(q1);
  //   std::this_thread::sleep_for(std::chrono::seconds(1));
  // if (id == 1) {
  //   BankAccountCall w1 = BankAccountCall("withdraw", "5");
  //   protocol.request(w1);
  // }
  // std::this_thread::sleep_for(std::chrono::seconds(1));

  std::thread reader([&] {
    std::string line;
    while (std::getline(std::cin, line)) {
      int x = std::stoi(line);
      std::string seq = std::to_string(id) + "-" + std::to_string(i++);
      if (x == 0) {
        BankAccountCall q1 = BankAccountCall(seq, "query", "");
        protocol.request(q1, true, false);
      } else if (x == 1) {
        BankAccountCall d1 = BankAccountCall(seq, "deposit", "10");
        protocol.request(d1, true, false);
      } else if (x == 2) {
        BankAccountCall d1 = BankAccountCall(seq, "withdraw", "10");
        protocol.request(d1, true, false);
      }
    }
  });
  reader.join();

  return 0;
}
