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

class Shop : public ReplicatedObject {
 public:
  enum MethodType { ADD = 0, REMOVE = 1, QUERY = 2 };

  Shop() {
    read_methods.push_back(static_cast<int>(MethodType::QUERY));
    update_methods.push_back(static_cast<int>(MethodType::ADD));
    update_methods.push_back(static_cast<int>(MethodType::REMOVE));
    method_args.insert({static_cast<int>(MethodType::ADD), 2});
    method_args.insert({static_cast<int>(MethodType::REMOVE), 1});
    method_args.insert({static_cast<int>(MethodType::QUERY), 0});
  }

  Shop(Shop& obj) : ReplicatedObject(obj) {
    std::scoped_lock lock(obj.mutex);
    active = obj.active;
    tombstones = obj.tombstones;
    num_adds = obj.num_adds;
  }

  std::string execute(MethodCall call) override {
    switch (static_cast<MethodType>(call.method_type)) {
      case MethodType::ADD: {
        auto separator = call.arg.find('-');
        if (separator == std::string::npos) {
          throw std::runtime_error("Malformed shop add");
        }
        int item = std::stoi(call.arg.substr(0, separator));
        int quantity = std::stoi(call.arg.substr(separator + 1));
        if (quantity <= 0) throw std::runtime_error("Invalid shop quantity");

        std::scoped_lock lock(mutex);
        std::string tag =
            std::to_string(self) + "." + std::to_string(++num_adds);
        auto found = active.find(item);
        return tag + "|" +
               (found == active.end() ? std::string()
                                      : encodeTags(found->second));
      }
      case MethodType::REMOVE: {
        int item = std::stoi(call.arg);
        std::scoped_lock lock(mutex);
        auto found = active.find(item);
        return found == active.end() ? std::string() : encodeTags(found->second);
      }
      case MethodType::QUERY: {
        std::scoped_lock lock(mutex);
        return std::to_string(active.size());
      }
    }
    throw std::runtime_error("Unknown shop method");
  }

  ReplicatedObject* executeDownstream(MethodCall call, bool) override {
    auto method = static_cast<MethodType>(call.method_type);
    if (method == MethodType::QUERY) return this;

    auto first_separator = call.arg.find('-');
    int item = std::stoi(call.arg.substr(0, first_separator));
    if (method == MethodType::REMOVE) {
      if (first_separator == std::string::npos) return this;
      applyRemove(item, decodeTagNames(call.arg.substr(first_separator + 1)));
      return this;
    }

    auto second_separator = call.arg.find('-', first_separator + 1);
    if (first_separator == std::string::npos ||
        second_separator == std::string::npos) {
      throw std::runtime_error("Malformed downstream shop add");
    }
    int quantity = std::stoi(call.arg.substr(
        first_separator + 1, second_separator - first_separator - 1));
    std::string metadata = call.arg.substr(second_separator + 1);
    auto metadata_separator = metadata.find('|');
    if (quantity <= 0 || metadata_separator == std::string::npos) {
      throw std::runtime_error("Malformed shop add metadata");
    }

    std::string tag = metadata.substr(0, metadata_separator);
    auto observed = decodeTagNames(metadata.substr(metadata_separator + 1));
    std::scoped_lock lock(mutex);
    removeObserved(item, observed);
    auto removed_item = tombstones.find(item);
    if (removed_item == tombstones.end() ||
        removed_item->second.find(tag) == removed_item->second.end()) {
      active[item][std::move(tag)] = quantity;
    }
    return this;
  }

  void toString() override {
    std::scoped_lock lock(mutex);
    int64_t total_quantity = 0;
    uint64_t digest = 0;
    for (auto const& [item, versions] : active) {
      int64_t quantity = 0;
      for (auto const& [tag, value] : versions) quantity += value;
      total_quantity += quantity;
      state_digest::addUnordered(
          digest, state_digest::mix(item) ^ state_digest::mix(quantity));
    }
    std::cout << "size: " << active.size() << std::endl;
    std::cout << "total quantity: " << total_quantity << std::endl;
    std::cout << "state digest: " << digest << std::endl;
  }

  bool isPermissible(MethodCall) override { return true; }

 private:
  static std::string encodeTags(std::map<std::string, int> const& versions) {
    std::string encoded;
    for (auto const& [tag, quantity] : versions) {
      if (!encoded.empty()) encoded += ',';
      encoded += tag;
    }
    return encoded;
  }

  static std::set<std::string> decodeTagNames(std::string const& encoded) {
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

  void applyRemove(int item, std::set<std::string> const& removed) {
    std::scoped_lock lock(mutex);
    removeObserved(item, removed);
  }

  void removeObserved(int item, std::set<std::string> const& removed) {
    if (removed.empty()) return;
    auto& item_tombstones = tombstones[item];
    auto found = active.find(item);
    for (auto const& tag : removed) {
      item_tombstones.insert(tag);
      if (found != active.end()) found->second.erase(tag);
    }
    if (found != active.end() && found->second.empty()) active.erase(found);
  }

  std::mutex mutex;
  std::map<int, std::map<std::string, int>> active;
  std::map<int, std::set<std::string>> tombstones;
  uint64_t num_adds = 0;
};
