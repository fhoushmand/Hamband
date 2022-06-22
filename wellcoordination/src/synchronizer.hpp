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

/*
  Here is the structure of a method call on the memory when we serialize and
  sent it: (total_length)|uint64_t| + (id_length)|uint64_t| +
  (call_length)|uint64_t| +

*/
struct MethodCall {
  std::string id;
  int method_type;
  std::string arg;

  std::vector<uint8_t> payload_buffer;
  uint8_t* payload;
  size_t length;

  int** dependency_vectors;

  size_t len;

  MethodCall() {}


  MethodCall(std::string id, int method_type, std::string arg) {
    this->id = id;
    this->method_type = method_type;
    this->arg = arg;
    // payload_buffer.resize(256);
    // payload = &payload_buffer[0];
  }

  void setDependencies(int** dependencies) { this->dependency_vectors = dependencies; }

};

struct pair_hash {
    inline std::size_t operator()(const std::pair<std::string,std::string> & v) const {
        std::hash<std::string> hasher;
        return hasher(v.first)*31+hasher(v.second);
    }
};


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
