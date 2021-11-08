#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

#include "replicated_object.hpp"

class Synchronizer
{
private:
    /* data */
public:
    Synchronizer(){}
    ~Synchronizer(){}

    virtual void request(std::string call, bool debug, bool summarize) = 0;
    virtual void response(MethodCall call, int res, bool debug) = 0;
};
