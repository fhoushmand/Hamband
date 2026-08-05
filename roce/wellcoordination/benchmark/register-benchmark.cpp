#include <cstdlib>
#include <string>

#include "register.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "register");
  std::srand(1);
  for (int call = 0; call < workload.writes(); call++) {
    workload.addWrite(0, Register::WRITE, std::to_string(std::rand() % 5));
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(Register::QUERY);
  }
  workload.finish();
  return 0;
}
