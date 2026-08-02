#include <thread>
#include "beb.hpp"
#include "account.hpp"
using namespace dory;
using namespace band;

void read(ReliableBroadcast& sender, size_t i);

void read(ReliableBroadcast& sender, size_t i) {
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

  ReplicatedObject* object = new BankAccount(100000);
  object->setID(id)->setNumProcess(nr_procs)->finalize();
  object->dependency_relation.clear();

  ReliableBroadcast* beb_instance = new ReliableBroadcast(id, remote_ids, object);
  std::cout << "Waiting..."
            << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  std::vector<uint8_t> payload_buffer(256);
  uint8_t* payload = &payload_buffer[0];
  for (int i = 0; i < 1; i++) {
    MethodCall c;
    std::string sequence_number =
            std::to_string(id) + "-" + std::to_string(i);
    if (id == 1)
      c = ReplicatedObject::createCall(sequence_number, "1 " + std::to_string(10 + i));
    else if (id == 2)
      c = ReplicatedObject::createCall(sequence_number, "1 " + std::to_string(20 + i));
    else
      c = ReplicatedObject::createCall(sequence_number, "1 " + std::to_string(30 + i));
    size_t s = object->serialize(c, payload);
    payload_buffer.resize(s);
    beb_instance->broadcast(payload, s, false);
  }
  
  std::this_thread::sleep_for(std::chrono::seconds(60));
  std::cout << "end\n";
  return 0;
}