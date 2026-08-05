#include <cstdlib>
#include <string>

#include "shop.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "shop");
  std::srand(1);
  int item = 0;
  for (int call = 0; call < workload.writes(); call++) {
    if (call % 2 == 0) {
      item = std::rand() % 1000000;
      int quantity = 1 + std::rand() % 10;
      workload.addWrite(0, Shop::ADD,
                        std::to_string(item) + "-" +
                            std::to_string(quantity));
    } else {
      workload.addWrite(0, Shop::REMOVE, std::to_string(item));
    }
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(Shop::QUERY);
  }
  workload.finish(false);
  return 0;
}
