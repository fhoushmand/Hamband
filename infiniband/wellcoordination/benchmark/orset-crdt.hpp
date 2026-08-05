#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>

#include "../src/replicated_object_crdt.hpp"
#include "state_digest.hpp"

class ORSet : public ReplicatedObject {
 public:
  enum MethodType { ADD = 0, REMOVE = 1, QUERY = 2 };

  ORSet() {
    read_methods.push_back(static_cast<int>(MethodType::QUERY));
    update_methods.push_back(static_cast<int>(MethodType::ADD));
    update_methods.push_back(static_cast<int>(MethodType::REMOVE));
    method_args.insert({static_cast<int>(MethodType::ADD), 1});
    method_args.insert({static_cast<int>(MethodType::REMOVE), 1});
    method_args.insert({static_cast<int>(MethodType::QUERY), 0});
  }

  ORSet(ORSet& obj) : ReplicatedObject(obj) {
    std::scoped_lock lock(obj.mutex);
    active = obj.active;
    tombstones = obj.tombstones;
    num_adds = obj.num_adds;
  }

  std::string execute(MethodCall call) override {
    switch (static_cast<MethodType>(call.method_type)) {
      case MethodType::ADD: {
        std::scoped_lock lock(mutex);
        return std::to_string(self) + "." + std::to_string(++num_adds);
      }
      case MethodType::REMOVE: {
        int element = std::stoi(call.arg);
        std::scoped_lock lock(mutex);
        auto found = active.find(element);
        if (found == active.end()) return "";
        return encodeTags(found->second);
      }
      case MethodType::QUERY: {
        std::scoped_lock lock(mutex);
        return std::to_string(active.size());
      }
    }
    throw std::runtime_error("Unknown OR-set method");
  }

  ReplicatedObject* executeDownstream(MethodCall call, bool) override {
    auto method = static_cast<MethodType>(call.method_type);
    if (method == MethodType::QUERY) return this;

    auto separator = call.arg.find('-');
    int element = std::stoi(call.arg.substr(0, separator));
    if (method == MethodType::ADD) {
      if (separator == std::string::npos) {
        throw std::runtime_error("Malformed OR-set add");
      }
      std::string tag = call.arg.substr(separator + 1);
      std::scoped_lock lock(mutex);
      auto removed = tombstones.find(element);
      if (removed == tombstones.end() ||
          removed->second.find(tag) == removed->second.end()) {
        active[element].insert(std::move(tag));
      }
      return this;
    }

    if (separator == std::string::npos) return this;
    auto removed = decodeTags(call.arg.substr(separator + 1));
    std::scoped_lock lock(mutex);
    auto& element_tombstones = tombstones[element];
    auto found = active.find(element);
    for (auto const& tag : removed) {
      element_tombstones.insert(tag);
      if (found != active.end()) found->second.erase(tag);
    }
    if (found != active.end() && found->second.empty()) active.erase(found);
    return this;
  }

  void toString() override {
    std::scoped_lock lock(mutex);
    uint64_t digest = 0;
    for (auto const& [element, tags] : active) {
      if (!tags.empty()) {
        state_digest::addUnordered(digest, state_digest::mix(element));
      }
    }
    std::cout << "size: " << active.size() << std::endl;
    std::cout << "state digest: " << digest << std::endl;
  }

  bool isPermissible(MethodCall) override { return true; }

 private:
  static std::string encodeTags(std::set<std::string> const& tags) {
    std::string encoded;
    for (auto const& tag : tags) {
      if (!encoded.empty()) encoded += ',';
      encoded += tag;
    }
    return encoded;
  }

  static std::set<std::string> decodeTags(std::string const& encoded) {
    std::set<std::string> tags;
    size_t begin = 0;
    while (begin < encoded.size()) {
      size_t end = encoded.find(',', begin);
      tags.insert(encoded.substr(begin, end - begin));
      if (end == std::string::npos) break;
      begin = end + 1;
    }
    return tags;
  }

  std::mutex mutex;
  std::map<int, std::set<std::string>> active;
  std::map<int, std::set<std::string>> tombstones;
  uint64_t num_adds = 0;
};
