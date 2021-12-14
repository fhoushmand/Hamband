#include <thread>
#include "beb.hpp"
using namespace dory;
using namespace hamsaz;

void read(BE_Broadcast& sender, size_t i);

void read(BE_Broadcast& sender, size_t i) {
  while (sender.partIter.hasNext(i)) {
    ParsedCall pslot = ParsedCall(sender.partIter.location(i));
    auto [buf, len] = pslot.payload();
    if (!pslot.isPopulated()) break;
    std::cout << "buf: " << buf << std::endl;
    sender.partIter.next(i);
  }
  return;
}

int main(int argc, char* argv[]) {
  LOGGER_DECL(logger);
  // LOGGER_INIT(logger, ConsensusConfig::logger_prefix);
  if (argc < 2) {
    throw std::runtime_error("Provide the id of the process as argument");
  }

  constexpr int nr_procs = 3;
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
  for (int i = 1; i <= nr_procs; i++) {
    if (id == i) {
      continue;
    } else {
      remote_ids.push_back(i);
    }
  }
  // std::string payload;
  BE_Broadcast* beb_instance = new BE_Broadcast(id, remote_ids);
  std::cout << "Waiting (5 sec) for all processes to fetch the connections"
            << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  std::vector<uint8_t> payload_buffer(8 + 2);
  uint8_t* payload = &payload_buffer[0];
  for (int i = 0; i < 1; i++) {
    if (id == 1)
      strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "1-");
    else if (id == 2)
      strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "2-");
    else
      strcpy(reinterpret_cast<char*>(const_cast<uint8_t*>(payload)), "3-");
    if(id == 1)
      beb_instance->broadcastCall(payload, 10, true);
  }

  if(id == 2 || id == 3){
    std::vector<uint8_t> payload_buffer2(100);
    uint8_t* local_memory = &payload_buffer2[0];
    std::this_thread::sleep_for(std::chrono::seconds(3));
    beb_instance->ce->connections().at(1).postSendSingle(
            ReliableConnection::RdmaRead,
            quorum::pack(quorum::EntryRd, id, 10),
            local_memory,
            100, 
            beb_instance->ce->connections().at(1).remoteBuf());
    beb_instance->pollCQ();
    std::cout << "reread: " << local_memory << std::endl;
  }

  
  std::thread reader([&beb_instance] { read(*beb_instance, 0); });
  reader.join();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::cout << "end\n";
  return 0;
}