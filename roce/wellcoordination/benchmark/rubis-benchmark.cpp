#include <cstdlib>
#include <string>

#include "rubis.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "rubis");
  std::srand(1);
  for (int call = 0; call < workload.writes(); call++) {
    int method = call % 6;
    std::string argument;
    switch (method) {
      case Rubis::SELL_ITEM:
        argument = std::to_string(100 + std::rand() % 100) + "-" +
                   std::to_string(std::rand() % 1000);
        break;
      case Rubis::STORE_BUY_NOW:
        argument = std::to_string(std::rand() % 100) + "-" +
                   std::to_string(std::rand() % 5);
        break;
      case Rubis::REGISTER_USER:
        argument = std::to_string(100 + std::rand() % 100);
        break;
      case Rubis::PLACE_BID:
        argument = std::to_string(std::rand() % 100) + "-" +
                   std::to_string(std::rand() % 100) + "-" +
                   std::to_string(std::rand() % 1000);
        break;
      case Rubis::OPEN_AUCTION:
        argument = std::to_string(100 + std::rand() % 100) + "-" +
                   std::to_string(1 + std::rand() % 1000);
        break;
      default:
        argument = std::to_string(std::rand() % 100);
        break;
    }
    workload.addWrite(0, method, argument);
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(Rubis::QUERY);
  }
  workload.finish();
  return 0;
}
