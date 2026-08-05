#include <cstdlib>
#include <string>

#include "pnset.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "pnset");
  std::srand(1);
  for (int call = 0; call < workload.writes(); call++) {
    int method = std::rand() % 2 == 0 ? PNSet::ADD : PNSet::REMOVE;
    workload.addWrite(call % workload.nodes(), method,
                      std::to_string(std::rand() % 100000));
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(PNSet::QUERY);
  }
  workload.finish();
  return 0;
}
