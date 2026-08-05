#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class WorkloadBuilder {
 public:
  WorkloadBuilder(int argc, char* argv[], std::string usecase)
      : usecase_(std::move(usecase)) {
    if (argc != 4) {
      throw std::runtime_error(
          "Usage: <generator> <nodes> <operations> <write-percent>");
    }
    nodes_ = std::stoi(argv[1]);
    operations_ = std::stoi(argv[2]);
    write_percentage_ = std::stoi(argv[3]);
    if (nodes_ < 2 || operations_ < 0 || write_percentage_ < 0 ||
        write_percentage_ > 100) {
      throw std::runtime_error("Invalid workload arguments");
    }
    writes_ = static_cast<int>(
        (static_cast<int64_t>(operations_) * write_percentage_) / 100);
    calls_.resize(nodes_);
  }

  int nodes() const { return nodes_; }
  int operations() const { return operations_; }
  int writes() const { return writes_; }
  int reads() const { return operations_ - writes_; }

  void addWrite(int node, int method, std::string const& argument = {}) {
    add(node, method, argument);
    writes_issued_++;
  }

  void addRead(int method, std::string const& argument = {}) {
    add(leastLoadedNode(), method, argument);
  }

  int followerFor(int sequence) const {
    return 1 + sequence % (nodes_ - 1);
  }

  void finish(bool shuffle = true) {
    size_t total_calls = 0;
    for (auto const& node_calls : calls_) {
      total_calls += node_calls.size();
    }
    if (writes_issued_ != writes_) {
      throw std::runtime_error("Generator emitted the wrong number of writes");
    }
    if (total_calls != static_cast<size_t>(operations_)) {
      throw std::runtime_error("Generator emitted the wrong operation count");
    }

    char const* configured_root = std::getenv("HAMBAND_WORKLOAD_DIR");
    std::filesystem::path root = configured_root == nullptr
                                     ? "/scratch/user/u.js213354/Hamband/"
                                       "wellcoordination/workload"
                                     : configured_root;
    std::filesystem::path directory =
        root / (std::to_string(nodes_) + "-" + std::to_string(operations_) +
                "-" + std::to_string(write_percentage_)) /
        usecase_;
    std::filesystem::create_directories(directory);

    for (int node = 0; node < nodes_; node++) {
      if (shuffle) {
        std::mt19937 random_engine(0x48414d42u + node);
        std::shuffle(calls_[node].begin(), calls_[node].end(), random_engine);
      }
      std::ofstream output(directory / (std::to_string(node + 1) + ".txt"),
                           std::ios::trunc);
      if (!output.is_open()) {
        throw std::runtime_error("Cannot create workload output file");
      }
      output << '#' << writes_ << '\n';
      for (auto const& call : calls_[node]) {
        output << call << '\n';
      }
    }

    std::cout << "generated " << operations_ << " operations (" << writes_
              << " writes) in " << directory << std::endl;
  }

 private:
  void add(int node, int method, std::string const& argument) {
    if (node < 0 || node >= nodes_) {
      throw std::runtime_error("Generator selected an invalid node");
    }
    std::string call = std::to_string(method);
    if (!argument.empty()) {
      call += " " + argument;
    }
    calls_[node].push_back(std::move(call));
  }

  int leastLoadedNode() const {
    return static_cast<int>(std::min_element(
                                calls_.begin(), calls_.end(),
                                [](auto const& left, auto const& right) {
                                  return left.size() < right.size();
                                }) -
                            calls_.begin());
  }

  std::string usecase_;
  int nodes_ = 0;
  int operations_ = 0;
  int write_percentage_ = 0;
  int writes_ = 0;
  int writes_issued_ = 0;
  std::vector<std::vector<std::string>> calls_;
};
