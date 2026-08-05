#include <cstdlib>
#include <string>

#include "orset.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "orset");
  std::srand(1);
  int element = 0;
  for (int call = 0; call < workload.writes(); call++) {
    int method = call % 2 == 0 ? ORSet::ADD : ORSet::REMOVE;
    if (method == ORSet::ADD) {
      element = std::rand() % 1000000;
    }
    workload.addWrite(0, method, std::to_string(element));
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(ORSet::QUERY);
  }
  workload.finish(false);
  return 0;
}
