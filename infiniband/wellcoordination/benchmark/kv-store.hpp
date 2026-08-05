#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/replicated_object_crdt.hpp"
#include "state_digest.hpp"

class KvStore : public ReplicatedObject {
 public:
  enum MethodType { PUT = 0, GET = 1 };
  static constexpr size_t KeyCount = 1000000;

  KvStore() : entries(KeyCount) {
    read_methods.push_back(static_cast<int>(MethodType::GET));
    update_methods.push_back(static_cast<int>(MethodType::PUT));
    method_args.insert({static_cast<int>(MethodType::PUT), 2});
    method_args.insert({static_cast<int>(MethodType::GET), 1});
  }

  std::string execute(MethodCall call) override {
    switch (static_cast<MethodType>(call.method_type)) {
      case MethodType::PUT: {
        auto separator = call.arg.find('-');
        if (separator == std::string::npos) {
          throw std::runtime_error("Malformed key-value put");
        }
        int key = std::stoi(call.arg.substr(0, separator));
        checkKey(key);
        uint32_t current_version =
            versionOf(entries[static_cast<size_t>(key)].load(
                std::memory_order_relaxed));
        uint32_t counter = (current_version >> 8) + 1;
        if (counter >= (1u << 24) || self < 0 || self > 255) {
          throw std::runtime_error("Key-value version exhausted");
        }
        return std::to_string(counter) + ":" + std::to_string(self);
      }
      case MethodType::GET: {
        int key = std::stoi(call.arg);
        checkKey(key);
        return std::to_string(valueOf(entries[static_cast<size_t>(key)].load(
            std::memory_order_relaxed)));
      }
    }
    throw std::runtime_error("Unknown key-value method");
  }

  ReplicatedObject* executeDownstream(MethodCall call, bool) override {
    if (static_cast<MethodType>(call.method_type) != MethodType::PUT) {
      return this;
    }

    auto first_separator = call.arg.find('-');
    auto last_separator = call.arg.rfind('-');
    auto version_separator = call.arg.find(':', last_separator + 1);
    if (first_separator == std::string::npos ||
        last_separator == first_separator ||
        version_separator == std::string::npos) {
      throw std::runtime_error("Malformed downstream key-value put");
    }

    int key = std::stoi(call.arg.substr(0, first_separator));
    int value = std::stoi(call.arg.substr(first_separator + 1,
                                          last_separator - first_separator - 1));
    uint32_t counter = static_cast<uint32_t>(
        std::stoul(call.arg.substr(last_separator + 1,
                                   version_separator - last_separator - 1)));
    uint32_t writer =
        static_cast<uint32_t>(std::stoul(call.arg.substr(version_separator + 1)));
    checkKey(key);
    if (counter >= (1u << 24) || writer > 255) {
      throw std::runtime_error("Invalid key-value version");
    }

    uint32_t version = (counter << 8) | writer;
    uint64_t desired = (static_cast<uint64_t>(version) << 32) |
                       static_cast<uint32_t>(value);
    auto& entry = entries[static_cast<size_t>(key)];
    uint64_t current = entry.load(std::memory_order_relaxed);
    while (versionOf(current) < version &&
           !entry.compare_exchange_weak(current, desired,
                                        std::memory_order_relaxed,
                                        std::memory_order_relaxed)) {
    }
    return this;
  }

  void toString() override {
    size_t written_keys = 0;
    uint64_t digest = 0;
    for (size_t key = 0; key < entries.size(); ++key) {
      uint64_t snapshot = entries[key].load(std::memory_order_relaxed);
      if (versionOf(snapshot) != 0) {
        written_keys++;
        state_digest::addUnordered(
            digest, state_digest::mix(key) ^ state_digest::mix(snapshot));
      }
    }
    std::cout << "written keys: " << written_keys << std::endl;
    std::cout << "state digest: " << digest << std::endl;
  }

  bool isPermissible(MethodCall) override { return true; }

 private:
  static uint32_t versionOf(uint64_t packed) {
    return static_cast<uint32_t>(packed >> 32);
  }

  static int valueOf(uint64_t packed) {
    return static_cast<int32_t>(static_cast<uint32_t>(packed));
  }

  static void checkKey(int key) {
    if (key < 0 || static_cast<size_t>(key) >= KeyCount) {
      throw std::out_of_range("Key-value key is out of range");
    }
  }

  std::vector<std::atomic<uint64_t>> entries;
};
