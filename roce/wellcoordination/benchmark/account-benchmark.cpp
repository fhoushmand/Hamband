#include <cstdlib>
#include <stdexcept>
#include <string>

#include "account.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "account");
  if (static_cast<int64_t>((workload.writes() + 1) / 2) * 4 >
      BankAccount::InitialBalance) {
    throw std::runtime_error("Account workload can exceed the initial balance");
  }

  std::srand(1);
  int deposits = 0;
  for (int call = 0; call < workload.writes(); call++) {
    int method = call % 2;
    int amount = method == BankAccount::WITHDRAW ? 1 + std::rand() % 4
                                                 : std::rand() % 5;
    int node = method == BankAccount::WITHDRAW
                   ? 0
                   : workload.followerFor(deposits++);
    workload.addWrite(node, method, std::to_string(amount));
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(BankAccount::QUERY);
  }
  workload.finish();
  return 0;
}
