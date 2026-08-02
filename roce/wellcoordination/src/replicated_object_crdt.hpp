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

#include "synchronizer.hpp"

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
    for (int i = 0; i < num_methods; i++){
      calls_applied[i] = new std::atomic<int>[num_process];
    }
    for (int x = 0; x < num_methods; x++)
      for (size_t i = 0; i < num_process; i++) {
        calls_applied[x][i] = 0;
      }
    return this;
  }

  virtual std::string execute(MethodCall call) = 0;
  virtual ReplicatedObject* executeDownstream(MethodCall call, bool b) = 0;
  virtual bool isPermissible(MethodCall call) = 0;
  virtual void toString() = 0;

  std::string internalExecuteCRDT(MethodCall call, size_t origin){
    std::string out = execute(call);
    // calls_applied_crdt[call.method_type][origin]++;
    return out;
  }

  void internalDownstreamExecuteCRDT(MethodCall call, size_t origin , bool b){
    executeDownstream(call, b);
    calls_applied[call.method_type][origin]++;
  }


  static MethodCall createCall(std::string id, std::string call) {
    int method_type;
    size_t spaceIndex = call.find_first_of(' ');
    if (spaceIndex == std::string::npos)
      method_type = std::stoi(call);
    else
      method_type = std::stoi(call.substr(0, spaceIndex));

    std::string arg = call.substr(spaceIndex + 1, call.size());

    MethodCall c = MethodCall(id, method_type, arg);
    return c;
  }

  std::string toString(MethodCall call) {
    return "(" + call.id + ")" + ":" + std::to_string(call.method_type) + " " + call.arg;
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

  static size_t serializeCRDT(MethodCall call, uint8_t* buffer) {
    std::vector<uint8_t> idVector(call.id.begin(), call.id.end());
    uint8_t* id_bytes = &idVector[0];
    uint64_t id_len = idVector.size();

    std::vector<uint8_t> argVector(call.arg.begin(), call.arg.end());
    uint8_t* arg_bytes = &argVector[0];
    uint64_t arg_len = argVector.size();

    // std::cout << "arg: " <<  call.arg << std::endl;


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

    
    uint64_t len = start - temp;

    auto length = reinterpret_cast<uint64_t*>(start - len - sizeof(uint64_t));
    *length = len;
    return len + sizeof(uint64_t) + 2 * sizeof(uint64_t);
  }

  MethodCall deserializeCRDT(uint8_t* buffer) {
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
    MethodCall output = MethodCall(id, method_type, arg);
    return output;
  }

  
  int getSynchGroup(int method_type){
    for (size_t i = 0; i < synch_groups.size(); i++){
      if(std::find(synch_groups[i].begin(), synch_groups[i].end(), method_type) != synch_groups[i].end())
        return static_cast<int>(i);
    }
    return -1;
  }

  static void parseArgsHelper(std::string parsedArgs[], std::string arg, int numArgs, int i){
    if(numArgs == 0)
      return;
    if(numArgs == 1){
      parsedArgs[i] = arg;
      return;
    }
    int index = arg.find_first_of('-');
    parsedArgs[i] = arg.substr(0, index);
    parseArgsHelper(parsedArgs, arg.substr(index + 1, arg.length()), numArgs--, i++);
  }

  static std::string* parseArgs(std::string arg, int numArgs){
    std::string* parsedArgs = new std::string[numArgs];
    parseArgsHelper(parsedArgs, arg, numArgs, 0);
    return parsedArgs;
  }

};