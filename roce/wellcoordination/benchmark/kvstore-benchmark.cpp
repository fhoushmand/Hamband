#include <cstdlib>
#include <string>

#include "kv-store.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "kvstore");
  std::srand(1);

  for (int call = 0; call < workload.writes(); ++call) {
    std::string key = std::to_string(std::rand() % KvStore::KeyCount);
    std::string value = std::to_string(std::rand() % 1000000);
    workload.addWrite(call % workload.nodes(), KvStore::PUT,
                      key + "-" + value);
  }
  for (int call = 0; call < workload.reads(); ++call) {
    workload.addRead(KvStore::GET,
                     std::to_string(std::rand() % KvStore::KeyCount));
  }
  workload.finish();
  return 0;
}
