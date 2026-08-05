#include <cstdlib>
#include <string>

#include "project.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "project");
  std::srand(1);
  int employees = 0;
  for (int call = 0; call < workload.writes(); call++) {
    switch (call % 4) {
      case Project::ADD_PROJECT:
        workload.addWrite(0, Project::ADD_PROJECT,
                          std::to_string(1001 + std::rand() % 100));
        break;
      case Project::DELETE_PROJECT:
        workload.addWrite(0, Project::DELETE_PROJECT,
                          std::to_string(1001 + std::rand() % 100));
        break;
      case Project::WORKS_ON:
        workload.addWrite(0, Project::WORKS_ON,
                          std::to_string(std::rand() % 1000) + "-" +
                              std::to_string(std::rand() % 1000));
        break;
      default:
        workload.addWrite(workload.followerFor(employees++),
                          Project::ADD_EMPLOYEE,
                          std::to_string(1001 + std::rand() % 100));
        break;
    }
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(Project::QUERY);
  }
  workload.finish();
  return 0;
}
