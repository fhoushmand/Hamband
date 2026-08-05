#include <cstdlib>
#include <string>

#include "smallbank.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "smallbank");
  constexpr int AccountCount = 100000000;
  std::srand(1);
  int deposits = 0;
  for (int call = 0; call < workload.writes(); call++) {
    int method = call % 2;
    int account = std::rand() % AccountCount;
    int amount = method == SmallBank::WITHDRAW ? 1 + std::rand() % 4
                                               : std::rand() % 20;
    int node = method == SmallBank::WITHDRAW
                   ? 0
                   : workload.followerFor(deposits++);
    workload.addWrite(node, method,
                      std::to_string(account) + "-" + std::to_string(amount));
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(SmallBank::QUERY,
                     std::to_string(std::rand() % AccountCount));
  }
  workload.finish();
  return 0;
}
