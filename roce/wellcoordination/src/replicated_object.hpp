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
#include <stdexcept>

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

  size_t serializedSize(MethodCall const& call) const {
    auto dependencies = dependency_relation.find(call.method_type);
    size_t num_dependencies = dependencies == dependency_relation.end()
                                  ? 0
                                  : dependencies->second.size();
    return 3 * sizeof(uint64_t) + sizeof(int) + call.id.size() +
           call.arg.size() +
           num_process * num_dependencies * sizeof(int);
  }

  size_t serialize(MethodCall const& call, uint8_t* buffer) {
    uint64_t id_len = call.id.size();
    uint64_t arg_len = call.arg.size();
    size_t total_size = serializedSize(call);
    uint64_t body_size = total_size - sizeof(uint64_t);
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

    auto dependencies = dependency_relation.find(call.method_type);
    if (dependencies != dependency_relation.end()) {
      for (int dependency_method : dependencies->second) {
        for (size_t process = 0; process < num_process; process++) {
          int applied = calls_applied[dependency_method][process].load();
          memcpy(cursor, &applied, sizeof(applied));
          cursor += sizeof(applied);
        }
      }
    }
    return static_cast<size_t>(cursor - buffer);
  }


  MethodCall deserialize(uint8_t* buffer) {
    uint64_t body_size;
    uint64_t id_len;
    uint64_t arg_len;
    int method_type;
    memcpy(&body_size, buffer, sizeof(body_size));
    memcpy(&id_len, buffer + sizeof(uint64_t), sizeof(id_len));
    memcpy(&arg_len, buffer + 2 * sizeof(uint64_t), sizeof(arg_len));
    memcpy(&method_type, buffer + 3 * sizeof(uint64_t), sizeof(method_type));

    size_t fixed_size = 3 * sizeof(uint64_t) + sizeof(int);
    if (body_size < fixed_size - sizeof(uint64_t) + id_len + arg_len) {
      throw std::runtime_error("Malformed method-call payload");
    }

    size_t id_offset = fixed_size;
    size_t arg_offset = id_offset + id_len;
    std::string id(reinterpret_cast<char*>(buffer + id_offset), id_len);
    std::string arg(reinterpret_cast<char*>(buffer + arg_offset), arg_len);

    size_t dependencies_offset = arg_offset + arg_len;
    auto dependency_entry = dependency_relation.find(method_type);
    size_t dependency_count = dependency_entry == dependency_relation.end()
                                  ? 0
                                  : dependency_entry->second.size();
    size_t required_body = fixed_size - sizeof(uint64_t) + id_len + arg_len +
                           dependency_count * num_process * sizeof(int);
    if (body_size != required_body) {
      throw std::runtime_error("Method-call payload has an invalid length");
    }

    std::vector<std::vector<int>> dependency_vectors(
        dependency_count, std::vector<int>(num_process));
    for (size_t dependency = 0; dependency < dependency_count; dependency++) {
      for (size_t process = 0; process < num_process; process++) {
        memcpy(&dependency_vectors[dependency][process],
               buffer + dependencies_offset, sizeof(int));
        dependencies_offset += sizeof(int);
      }
    }
    MethodCall output(id, method_type, arg);
    output.setDependencies(std::move(dependency_vectors));
    return output;
  }

  MethodCall deserialize(char* buffer) {
    return deserialize(reinterpret_cast<uint8_t*>(buffer));
  }
};
