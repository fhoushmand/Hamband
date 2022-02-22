#pragma once 

#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <set>
#include <string>
#include <cstring>
#include <map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <atomic>

/*
  Here is the structure of a method call on the memory when we serialize and
  sent it: (total_length)|uint64_t| + (id_length)|uint64_t| +
  (call_length)|uint64_t| +

*/
struct MethodCall {
  // public
  std::string id;
  // std::string call;
  int method_type;
  // std::string method_name;
  std::string arg;

  int** dependency_vectors;

  size_t len;

  MethodCall() {}


  MethodCall(std::string id, int method_type, std::string arg) {
    this->id = id;
    this->method_type = method_type;
    this->arg = arg;
  }

  void setDependencies(int** dependencies) { this->dependency_vectors = dependencies; }

};


struct pair_hash {
    inline std::size_t operator()(const std::pair<std::string,std::string> & v) const {
        std::hash<std::string> hasher;
        return hasher(v.first)*31+hasher(v.second);
    }
};


class ReplicatedObject {
 private:
  /* data */
 public:
 
  int self;
  size_t num_process;
  int num_methods;
  std::vector<int> read_methods;
  std::vector<int> update_methods;
  std::vector<std::vector<int>> synch_groups;
  std::map<int, std::vector<int>> dependency_relation;
  std::map<int, int> method_args;

  std::atomic<int>** calls_applied;

  ReplicatedObject() {}
  ~ReplicatedObject() {}

  ReplicatedObject* setNumProcess(size_t num_procs){
    this->num_process = num_procs;
    return this;
  }

  ReplicatedObject* setID(int self){
    this->self = self;
    return this;
  }

  ReplicatedObject* finalize(){
    this->num_methods = static_cast<int>(read_methods.size() + update_methods.size());
    calls_applied = new std::atomic<int>*[num_methods];
    for (int i = 0; i < num_methods; i++)
      calls_applied[i] = new std::atomic<int>[num_process];
    for (int x = 0; x < num_methods; x++)
      for (size_t i = 0; i < num_process; i++) calls_applied[x][i] = 0;
    return this;
  }

  virtual ReplicatedObject* execute(MethodCall call) = 0;
  virtual bool isPermissible(MethodCall call) = 0;
  virtual void toString() = 0;


  void internalExecute(MethodCall call, size_t origin){
    execute(call);
    calls_applied[call.method_type][origin]++;
  }

  int getSynchGroup(int method_type){
    for (size_t i = 0; i < synch_groups.size(); i++){
      if(std::find(synch_groups[i].begin(), synch_groups[i].end(), method_type) != synch_groups[i].end())
        return static_cast<int>(i);
    }
    return -1;
  }

  static MethodCall createCall(std::string id, std::string call) {
    int method_type;
    size_t spaceIndex = call.find_first_of(' ');
    if (spaceIndex == std::string::npos)
      method_type = std::stoi(call);
    else
      method_type = std::stoi(call.substr(0, spaceIndex));

    std::string arg = call.substr(spaceIndex + 1, call.size());

    return MethodCall(id, method_type, arg);
  }


  void printCall(MethodCall call) {
    std::cout << "(" << call.id << ")" << ":" << call.method_type << " " << call.arg << std::endl;
    
    for (size_t x = 0; x < dependency_relation[call.method_type].size(); x++) {
      std::cout << dependency_relation[call.method_type][x] << " = {";
      for (size_t i = 0; i < num_process; i++)
      {
        if(i != num_process - 1)
          std::cout << call.dependency_vectors[x][i] << ", ";
        else
          std::cout << call.dependency_vectors[x][i];
      }
      std::cout << "}" << std::endl;
    }
  }

  size_t serialize(MethodCall call, uint8_t* buffer) {
    std::vector<uint8_t> idVector(call.id.begin(), call.id.end());
    uint8_t* id_bytes = &idVector[0];
    uint64_t id_len = idVector.size();

    // std::cout << "id bytes: " << id_bytes << std::endl;
    // std::cout << "id bytes size: " << id_len << std::endl;

    // if(!call.arg.empty()){
    std::vector<uint8_t> argVector(call.arg.begin(), call.arg.end());
    uint8_t* arg_bytes = &argVector[0];
    uint64_t arg_len = argVector.size();
    // }


    uint8_t* start = buffer + sizeof(uint64_t);
    auto temp = start;

    *reinterpret_cast<uint64_t*>(start) = id_len;
    start += sizeof(id_len);

    *reinterpret_cast<uint64_t*>(start) = arg_len;
    start += sizeof(arg_len);

    *reinterpret_cast<int*>(start) = call.method_type;
    start += sizeof(call.method_type);

    memcpy(start, id_bytes, id_len);
    start += id_len;

    if (arg_len != 0) {
      memcpy(start, arg_bytes, arg_len);
      start += arg_len;
    }

    size_t num_dependencies = dependency_relation[call.method_type].size();
    if (num_dependencies != 0) {
      // int** vector = new int*[dependency_size];
      for (size_t i = 0; i < num_dependencies; i++) {
        // int* vs = new int[num_process];
        for (size_t j = 0; j < num_process; j++){
          *reinterpret_cast<int*>(start) = calls_applied[dependency_relation[call.method_type][i]][j];
          start += sizeof(calls_applied[dependency_relation[call.method_type][i]][j]);
        }
      }
    }
    uint64_t len = start - temp;

    auto length = reinterpret_cast<uint64_t*>(start - len - sizeof(uint64_t));
    *length = len;
    return len + sizeof(uint64_t) + 2 * sizeof(uint64_t) +
           sizeof(int) + num_process * num_dependencies * sizeof(int);
  }



  MethodCall* deserialize(uint8_t* buffer) {
    uint64_t len = *reinterpret_cast<uint64_t*>(buffer);
    // std::cout << "-tot_size: " << len << std::endl;

    uint64_t id_len = *reinterpret_cast<uint64_t*>(buffer + sizeof(uint64_t));
    // std::cout << "-id_size: " << id_len << std::endl;

    uint64_t arg_len =
        *reinterpret_cast<uint64_t*>(buffer + 2 * sizeof(uint64_t));
    // std::cout << "-arg_size: " << arg_len << std::endl;

    int method_type =
        *reinterpret_cast<int*>(buffer + 3 * sizeof(uint64_t));
    // std::cout << "-method: " << method_type << std::endl;

    size_t id_offset = 3 * sizeof(uint64_t) + sizeof(int);
    uint8_t* id_bytes = new uint8_t[id_len];
    memcpy(id_bytes, buffer + id_offset, id_len);
    std::string id(id_bytes, id_bytes + id_len);

    // std::cout << "-id: " << id << std::endl;

    size_t arg_offset = id_offset + id_len;
    std::string arg = "";
    if(arg_len != 0)
    {
      uint8_t* arg_bytes = new uint8_t[arg_len];
      memcpy(arg_bytes, buffer + arg_offset, arg_len);
      arg = std::string(arg_bytes, arg_bytes + arg_len);
      // std::cout << "-arg: " << arg << std::endl;
    }
      
    
    size_t dependencies_offset = arg_offset + arg_len;
    std::vector<int> dependency_methods = dependency_relation[method_type];
    int** dependency_vectors = new int*[dependency_methods.size()];
    for (size_t i = 0; i < dependency_methods.size(); i++)
      dependency_vectors[i] = new int[num_process];

    
    for (size_t x = 0; x < dependency_methods.size(); x++) {
      // int method = dependency_methods[x];
      // std::cout << "-method: " << method << std::endl;
      for (size_t i = 0; i < num_process; i++) {
        dependency_vectors[x][i] = *reinterpret_cast<int*>(buffer + dependencies_offset + i * sizeof(int));
        // std::cout << dependency_vectors[x][i] << ", ";
      }
      dependencies_offset += num_process * sizeof(int);
    }
    MethodCall* output = new MethodCall(id, method_type, arg);
    output->setDependencies(dependency_vectors);
    return output;
  }
};

// class MethodCallFactory {
//  public:
//   uint64_t num_process;
//   // we have this in the object
//   std::map<int, std::vector<int>> dependency_relation;

//   MethodCallFactory() {}

//   MethodCallFactory(ReplicatedObject* obj, uint64_t r) {
//     this->dependency_relation = obj->dependency_relation;
//     this->num_process = r;
//   }

//   MethodCall createCall(std::string id, std::string call) {
//     int method_type;
//     size_t spaceIndex = call.find_first_of(' ');
//     if (spaceIndex == std::string::npos)
//       method_type = std::stoi(call);
//     else
//       method_type = std::stoi(call.substr(0, spaceIndex));

//     std::string arg = call.substr(spaceIndex + 1, call.size());

//     return MethodCall(id, method_type, arg);
//   }

//   size_t serialize(MethodCall call, uint8_t* buffer) {
//     std::vector<uint8_t> idVector(call.id.begin(), call.id.end());
//     uint8_t* id_bytes = &idVector[0];
//     uint64_t id_len = idVector.size();

//     // if(!call.arg.empty()){
//     std::vector<uint8_t> argVector(call.arg.begin(), call.arg.end());
//     uint8_t* arg_bytes = &argVector[0];
//     uint64_t arg_len = argVector.size();
//     // }

//     // std::cout << "call bytes: " << call_bytes << std::endl;
//     // std::cout << "call bytes size: " << call_len << std::endl;

//     uint8_t* start = buffer + sizeof(uint64_t);
//     auto temp = start;

//     *reinterpret_cast<uint64_t*>(start) = id_len;
//     start += sizeof(id_len);

//     *reinterpret_cast<uint64_t*>(start) = arg_len;
//     start += sizeof(arg_len);

//     *reinterpret_cast<int*>(start) = call.method_type;
//     start += sizeof(call.method_type);

//     memcpy(start, id_bytes, id_len);
//     start += id_len;

//     if (arg_len != 0) {
//       memcpy(start, arg_bytes, arg_len);
//       start += arg_len;
//     }


//     size_t num_dependencies = dependency_relation[call.method_type].size();
//     if (num_dependencies != 0) {
//       // int** vector = new int*[dependency_size];
//       for (size_t i = 0; i < dependency_size; i++) {
//         // int* vs = new int[num_process];
//         for (size_t j = 0; j < num_process; j++)
//           *reinterpret_cast<int*>(start) = calls_applied[dependency_relation[call.method_type][i]][j];
//           start += sizeof(calls_applied[dependency_relation[call.method_type][i]][j]);
//           // vs[j] = calls_applied[dependency_relation[call.method_type][i]][j];
//         // vector[i] = vs;
//       }
//       // dependency_vectors = vector;
//       // request.attachDependencies(vector);
//     }


//     // size_t num_dependencies = 0;
//     // if (dependency_relation.find(call.method_type) != dependency_relation.end()) {
//     //   num_dependencies = dependency_relation[call.method_type].size();
//     //   for (size_t x = 0; x < num_dependencies; x++) {
//     //     for (size_t i = 0; i < num_process; i++) {
//     //       *reinterpret_cast<int*>(start) = call.dependency_vectors[x][i];
//     //       start += sizeof(call.dependency_vectors[x][i]);
//     //     }
//     //   }
//     // }

//     uint64_t len = start - temp;
    
//     auto length = reinterpret_cast<uint64_t*>(start - len - sizeof(uint64_t));
//     *length = len;
//     return len + sizeof(uint64_t) + 2 * sizeof(uint64_t) +
//            sizeof(int) + num_process * num_dependencies * sizeof(int);
//   }

//   MethodCall* deserialize(uint8_t* buffer) {
//     uint64_t len = *reinterpret_cast<uint64_t*>(buffer);
//     // std::cout << "-tot_size: " << len << std::endl;

//     uint64_t id_len = *reinterpret_cast<uint64_t*>(buffer + sizeof(uint64_t));
//     // std::cout << "-id_size: " << id_len << std::endl;

//     uint64_t arg_len =
//         *reinterpret_cast<uint64_t*>(buffer + 2 * sizeof(uint64_t));
//     // std::cout << "-arg_size: " << arg_len << std::endl;

//     int method_type =
//         *reinterpret_cast<int*>(buffer + 3 * sizeof(uint64_t));
//     // std::cout << "-method: " << method_type << std::endl;

//     size_t id_offset = 3 * sizeof(uint64_t) + sizeof(int);
//     uint8_t* id_bytes = new uint8_t[id_len];
//     memcpy(id_bytes, buffer + id_offset, id_len);
//     std::string id(id_bytes, id_bytes + id_len);

//     // std::cout << "-id: " << id << std::endl;

//     size_t arg_offset = id_offset + id_len;
//     std::string arg = "";
//     if(arg_len != 0)
//     {
//       uint8_t* arg_bytes = new uint8_t[arg_len];
//       memcpy(arg_bytes, buffer + arg_offset, arg_len);
//       arg = std::string(arg_bytes, arg_bytes + arg_len);
//       // std::cout << "-arg: " << arg << std::endl;
//     }
      
    
//     size_t dependencies_offset = arg_offset + arg_len;
//     std::vector<int> dependency_methods = dependency_relation[method_type];
//     int** dependency_vectors = new int*[dependency_methods.size()];
//     for (size_t i = 0; i < dependency_methods.size(); i++)
//       dependency_vectors[i] = new int[num_process];

    
//     for (size_t x = 0; x < dependency_methods.size(); x++) {
//       // int method = dependency_methods[x];
//       // std::cout << "-method: " << method << std::endl;
//       for (size_t i = 0; i < num_process; i++) {
//         dependency_vectors[x][i] = *reinterpret_cast<int*>(buffer + dependencies_offset + i * sizeof(int));
//         // std::cout << dependency_vectors[x][i] << ", ";
//       }
//       dependencies_offset += num_process * sizeof(int);
//     }

//     MethodCall* out = new MethodCall(id, method_type, arg);
//     out->setDependencies(dependency_vectors);
//     return out;
//   }

//   void toString(MethodCall call) {
//     std::cout << "(" << call.id << ")" << ":" << call.method_type << " " << call.arg << std::endl;
    
//     for (size_t x = 0; x < dependency_relation[call.method_type].size(); x++) {
//       std::cout << dependency_relation[call.method_type][x] << " = {";
//       for (size_t i = 0; i < num_process; i++)
//       {
//         if(i != num_process - 1)
//           std::cout << call.dependency_vectors[x][i] << ", ";
//         else
//           std::cout << call.dependency_vectors[x][i];
//       }
//       std::cout << "}" << std::endl;
//     }
//   }

// };