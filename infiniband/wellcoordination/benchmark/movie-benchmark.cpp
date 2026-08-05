#include <cstdlib>
#include <string>

#include "movie.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "movie");
  std::srand(1);
  for (int call = 0; call < workload.writes(); call++) {
    int method = call % 4;
    int node = method <= Movie::REMOVE_MOVIE ? 0 : 1;
    workload.addWrite(node, method,
                      std::to_string(1001 + std::rand() % 100));
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(Movie::QUERY);
  }
  workload.finish();
  return 0;
}
