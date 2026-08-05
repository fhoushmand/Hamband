#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../src/replicated_object_crdt.hpp"
#include "state_digest.hpp"

class Register : public ReplicatedObject {
 public:
  enum MethodType { WRITE = 0, QUERY = 1 };

  Register() {
    read_methods.push_back(static_cast<int>(MethodType::QUERY));
    update_methods.push_back(static_cast<int>(MethodType::WRITE));
    method_args.insert({static_cast<int>(MethodType::WRITE), 1});
    method_args.insert({static_cast<int>(MethodType::QUERY), 0});
  }

  Register(Register& obj) : ReplicatedObject(obj) {
    state.store(obj.state.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
    clock.store(obj.clock.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
  }

  std::string execute(MethodCall call) override {
    switch (static_cast<MethodType>(call.method_type)) {
      case MethodType::WRITE: {
        auto separator = call.arg.find('-');
        std::stoi(call.arg.substr(0, separator));
        uint32_t counter = clock.fetch_add(1, std::memory_order_relaxed) + 1;
        if (counter >= (1u << 24) || self < 0 || self > 255) {
          throw std::runtime_error("Register version exhausted");
        }
        return std::to_string(counter) + ":" + std::to_string(self);
      }
      case MethodType::QUERY:
        return std::to_string(valueOf(state.load(std::memory_order_relaxed)));
    }
    throw std::runtime_error("Unknown register method");
  }

  ReplicatedObject* executeDownstream(MethodCall call, bool) override {
    if (static_cast<MethodType>(call.method_type) != MethodType::WRITE) {
      return this;
    }

    auto argument_separator = call.arg.find('-');
    auto version_separator = call.arg.find(':', argument_separator + 1);
    if (argument_separator == std::string::npos ||
        version_separator == std::string::npos) {
      throw std::runtime_error("Malformed register write");
    }

    int value = std::stoi(call.arg.substr(0, argument_separator));
    uint32_t counter = static_cast<uint32_t>(
        std::stoul(call.arg.substr(argument_separator + 1,
                                   version_separator - argument_separator - 1)));
    uint32_t writer =
        static_cast<uint32_t>(std::stoul(call.arg.substr(version_separator + 1)));
    if (counter >= (1u << 24) || writer > 255) {
      throw std::runtime_error("Invalid register version");
    }

    advanceClock(counter);
    uint32_t version = (counter << 8) | writer;
    uint64_t desired = (static_cast<uint64_t>(version) << 32) |
                       static_cast<uint32_t>(value);
    uint64_t current = state.load(std::memory_order_relaxed);
    while (versionOf(current) < version &&
           !state.compare_exchange_weak(current, desired,
                                        std::memory_order_relaxed,
                                        std::memory_order_relaxed)) {
    }
    return this;
  }

  void toString() override {
    uint64_t snapshot = state.load(std::memory_order_relaxed);
    uint32_t version = versionOf(snapshot);
    std::cout << "reg: " << valueOf(snapshot) << std::endl;
    std::cout << "version: " << (version >> 8) << ":" << (version & 0xff)
              << std::endl;
    std::cout << "state digest: " << state_digest::mix(snapshot) << std::endl;
  }

  bool isPermissible(MethodCall) override { return true; }

 private:
  static uint32_t versionOf(uint64_t packed) {
    return static_cast<uint32_t>(packed >> 32);
  }

  static int valueOf(uint64_t packed) {
    return static_cast<int32_t>(static_cast<uint32_t>(packed));
  }

  void advanceClock(uint32_t observed) {
    uint32_t current = clock.load(std::memory_order_relaxed);
    while (current < observed &&
           !clock.compare_exchange_weak(current, observed,
                                        std::memory_order_relaxed,
                                        std::memory_order_relaxed)) {
    }
  }

  std::atomic<uint64_t> state{0};
  std::atomic<uint32_t> clock{0};
};
