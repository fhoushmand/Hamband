#include <cstdlib>
#include <string>

#include "courseware.hpp"
#include "workload_builder.hpp"

int main(int argc, char* argv[]) {
  WorkloadBuilder workload(argc, argv, "courseware");
  std::srand(1);
  int students = 0;
  for (int call = 0; call < workload.writes(); call++) {
    switch (call % 4) {
      case Courseware::ADD_COURSE:
        workload.addWrite(0, Courseware::ADD_COURSE,
                          std::to_string(1001 + std::rand() % 100));
        break;
      case Courseware::DELETE_COURSE:
        workload.addWrite(0, Courseware::DELETE_COURSE,
                          std::to_string(1001 + std::rand() % 100));
        break;
      case Courseware::ENROLL:
        workload.addWrite(0, Courseware::ENROLL,
                          std::to_string(std::rand() % 1000) + "-" +
                              std::to_string(std::rand() % 1000));
        break;
      default:
        workload.addWrite(workload.followerFor(students++),
                          Courseware::ADD_STUDENT,
                          std::to_string(1001 + std::rand() % 100));
        break;
    }
  }
  for (int call = 0; call < workload.reads(); call++) {
    workload.addRead(Courseware::QUERY);
  }
  workload.finish();
  return 0;
}
