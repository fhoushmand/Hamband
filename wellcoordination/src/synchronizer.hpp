#pragma once 

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

#include "replicated_object.hpp"

enum class ResponseStatus {
  NoError = 0,  // Placeholder for the 0 value
  NotPermissible,
	DoryError
};

class Synchronizer
{
private:
    /* data */
public:
    Synchronizer(){}
    ~Synchronizer(){}

    virtual std::pair<ResponseStatus,std::chrono::high_resolution_clock::time_point> request(MethodCall call, bool debug, bool summarize) = 0;
    
    virtual std::pair<ResponseStatus,std::chrono::high_resolution_clock::time_point> response(MethodCall request, ResponseStatus response, bool debug) {
			auto end = std::chrono::high_resolution_clock::now();
			return std::make_pair(response, end);
  	}
};
