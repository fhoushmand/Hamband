#include <cstdlib>
#include <string>

#include "gset.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "gset");
  std::srand(1);
  for (int call = 0; call < workload.writes(); call++) {
    workload.addWrite(call % workload.nodes(), GSet::ADD,
                      std::to_string(std::rand() % 5));
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(GSet::QUERY);
  }
  workload.finish();
  return 0;
}
