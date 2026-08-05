#include <cstdlib>
#include <string>

#include "counter.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "counter");
  std::srand(1);
  for (int call = 0; call < workload.writes(); call++) {
    workload.addWrite(call % workload.nodes(), Counter::ADD,
                      std::to_string(std::rand() % 5));
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(Counter::QUERY);
  }
  workload.finish();
  return 0;
}
