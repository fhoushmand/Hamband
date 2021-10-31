// #include <thread>
// #include "beb.hpp"
// using namespace dory;
// using namespace hamsaz;

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
//     // sender.partIter.reset(i);
//   }

//   // }
// }

// int main(int argc, char* argv[]) {
//   LOGGER_DECL(logger);
//   // LOGGER_INIT(logger, ConsensusConfig::logger_prefix);
//   if (argc < 2) {
//     throw std::runtime_error("Provide the id of the process as argument");
//   }

//   constexpr int nr_procs = 3;
//   constexpr int minimum_id = 1;
//   int id = 0;
//   switch (argv[1][0]) {
//     case '1':
//       id = 1;
//       break;
//     case '2':
//       id = 2;
//       break;
//     case '3':
//       id = 3;
//       break;
//     case '4':
//       id = 4;
//       break;
//     case '5':
//       id = 5;
//       break;
//     default:
//       throw std::runtime_error("Invalid id");
//   }

//   // Build the list of remote ids
//   std::vector<int> remote_ids;
//   for (int i = 1; i <= nr_procs; i++) {
//     if (id == i) {
//       continue;
//     } else {
//       remote_ids.push_back(i);
//     }
//   }
//   // std::string payload;
//   BE_Broadcast* beb_instance = new BE_Broadcast(id, remote_ids);
//   std::cout << "Waiting (5 sec) for all processes to fetch the connections"
//             << std::endl;
//   std::this_thread::sleep_for(std::chrono::seconds(5));

//   std::vector<uint8_t> payload_buffer(8 + 2);
//   uint8_t* payload = &payload_buffer[0];
//   for (int i = 0; i < 5; i++) {
//     if (id == 1)
//       strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "1-");
//     else if (id == 2)
//       strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "2-");
//     else
//       strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "3-");
//     // int array[]{10*id, 20*id, 30*id};
//     // if(id == 1 )
//       beb_instance->broadcastCallNoDep(payload, 10, 1, true);
//   }

//   std::thread reader([&beb_instance] { read(*beb_instance); });
//   reader.join();
//   std::cout << "end\n";
//   return 0;
// }