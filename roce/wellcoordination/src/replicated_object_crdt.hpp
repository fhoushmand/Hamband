#pragma once 

#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <set>
#include <stdexcept>
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

    std::string arg = spaceIndex == std::string::npos
                          ? std::string()
                          : call.substr(spaceIndex + 1);

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

  static size_t serializedSizeCRDT(MethodCall const& call) {
    return 3 * sizeof(uint64_t) + sizeof(int) + call.id.size() +
           call.arg.size();
  }

  static size_t serializeCRDT(MethodCall const& call, uint8_t* buffer) {
    uint64_t id_len = call.id.size();
    uint64_t arg_len = call.arg.size();
    uint64_t body_size = serializedSizeCRDT(call) - sizeof(uint64_t);
    uint8_t* cursor = buffer;

    memcpy(cursor, &body_size, sizeof(body_size));
    cursor += sizeof(body_size);
    memcpy(cursor, &id_len, sizeof(id_len));
    cursor += sizeof(id_len);
    memcpy(cursor, &arg_len, sizeof(arg_len));
    cursor += sizeof(arg_len);
    memcpy(cursor, &call.method_type, sizeof(call.method_type));
    cursor += sizeof(call.method_type);
    memcpy(cursor, call.id.data(), id_len);
    cursor += id_len;
    memcpy(cursor, call.arg.data(), arg_len);
    cursor += arg_len;
    return static_cast<size_t>(cursor - buffer);
  }

  MethodCall deserializeCRDT(uint8_t* buffer) {
    uint64_t body_size;
    uint64_t id_len;
    uint64_t arg_len;
    int method_type;
    memcpy(&body_size, buffer, sizeof(body_size));
    memcpy(&id_len, buffer + sizeof(uint64_t), sizeof(id_len));
    memcpy(&arg_len, buffer + 2 * sizeof(uint64_t), sizeof(arg_len));
    memcpy(&method_type, buffer + 3 * sizeof(uint64_t), sizeof(method_type));
    uint64_t expected_body_size =
        2 * sizeof(uint64_t) + sizeof(int) + id_len + arg_len;
    if (body_size != expected_body_size) {
      throw std::runtime_error("Malformed CRDT payload length");
    }

    size_t id_offset = 3 * sizeof(uint64_t) + sizeof(int);
    std::string id(reinterpret_cast<char*>(buffer + id_offset), id_len);

    // std::cout << "-id: " << id << std::endl;

    size_t arg_offset = id_offset + id_len;
    std::string arg(reinterpret_cast<char*>(buffer + arg_offset), arg_len);
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
    size_t index = arg.find_first_of('-');
    if (index == std::string::npos) {
      parsedArgs[i] = arg;
      return;
    }
    parsedArgs[i] = arg.substr(0, index);
    parseArgsHelper(parsedArgs, arg.substr(index + 1), numArgs - 1, i + 1);
  }

  static std::string* parseArgs(std::string arg, int numArgs){
    std::string* parsedArgs = new std::string[numArgs];
    parseArgsHelper(parsedArgs, arg, numArgs, 0);
    return parsedArgs;
  }

};
