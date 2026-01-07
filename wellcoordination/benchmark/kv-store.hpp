
#include <atomic>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include "../src/replicated_object_crdt.hpp"

typedef unsigned char uint8_t;

class KvStore : public ReplicatedObject {
 private:
 public:
  enum MethodType { PUT = 0, GET = 1 };

  struct ValTsAtomic {
    std::atomic<int> value{0};
    std::atomic<int> ts{0};

    ValTsAtomic() = default;

    // No copying
    ValTsAtomic(const ValTsAtomic&) = delete;
    ValTsAtomic& operator=(const ValTsAtomic&) = delete;

    // Allow moving (vector needs this sometimes)
    ValTsAtomic(ValTsAtomic&& other) noexcept {
      value.store(other.value.load(std::memory_order_relaxed),
                  std::memory_order_relaxed);
      ts.store(other.ts.load(std::memory_order_relaxed),
               std::memory_order_relaxed);
    }
    ValTsAtomic& operator=(ValTsAtomic&& other) noexcept {
      if (this != &other) {
        value.store(other.value.load(std::memory_order_relaxed),
                    std::memory_order_relaxed);
        ts.store(other.ts.load(std::memory_order_relaxed),
                 std::memory_order_relaxed);
      }
      return *this;
    }
  };

  std::vector<ValTsAtomic> localkeysvalues;
  std::vector<ValTsAtomic> keysvalues;

  KvStore() : keysvalues(1000000), localkeysvalues(1000000) {
    // value = 0;

    read_methods.push_back(static_cast<int>(MethodType::GET));

    update_methods.push_back(static_cast<int>(MethodType::PUT));

    method_args.insert(std::make_pair(static_cast<int>(MethodType::PUT), 2));
    method_args.insert(std::make_pair(static_cast<int>(MethodType::GET), 1));
  }

  KvStore(const KvStore& obj) : ReplicatedObject(obj) {
    localkeysvalues.resize(obj.localkeysvalues.size());
    keysvalues.resize(obj.keysvalues.size());

    for (size_t i = 0; i < obj.localkeysvalues.size(); ++i) {
      localkeysvalues[i].value.store(obj.localkeysvalues[i].value.load());
      localkeysvalues[i].ts.store(obj.localkeysvalues[i].ts.load());
    }
    for (size_t i = 0; i < obj.keysvalues.size(); ++i) {
      keysvalues[i].value.store(obj.keysvalues[i].value.load());
      keysvalues[i].ts.store(obj.keysvalues[i].ts.load());
    }
  }

  // 0
  std::string put(int key, int val) {
    int local_timestamp = localkeysvalues[key].ts.load();
    int local_value = localkeysvalues[key].value.load();
    local_timestamp += 1;
    local_value = val;
    localkeysvalues[key].ts.store(local_timestamp);
    localkeysvalues[key].value.store(local_value);
    return std::to_string(local_timestamp);
  }

  // downStream
  void putDownstream(int key, int val, int ts) {
    if (keysvalues[key].ts.load() < ts) {
      keysvalues[key].ts.store(ts);
      keysvalues[key].value.store(val);
    }
  }
  // 1
  std::string get(int key) {
    // optional bounds check (recommended)
    if (key < 0 || (size_t)key >= keysvalues.size()) return "";

    int lts = localkeysvalues[key].ts.load();
    int gts = keysvalues[key].ts.load();

    if (lts > gts) {
      keysvalues[key].ts.store(lts);
      keysvalues[key].value.store(localkeysvalues[key].value.load());
    }

    // return the current value (you can also return "value-ts" if you want)
    return std::to_string(keysvalues[key].value.load());
  }

  virtual std::string execute(MethodCall call) {
    switch (static_cast<MethodType>(call.method_type)) {
      case MethodType::PUT: {
        size_t index = call.arg.find_first_of('-');
        std::string key = call.arg.substr(0, index);
        std::string value = call.arg.substr(index + 1, call.arg.length());
        return put(std::stoi(key), std::stoi(value));
        break;
      }
      case MethodType::GET:
        return get(std::stoi(call.arg));
        break;
      default:
        std::cout << "wrong method name" << std::endl;
        break;
    }
    return "";
  }

  virtual ReplicatedObject* executeDownstream(MethodCall call, bool b) {
    switch (static_cast<MethodType>(call.method_type)) {
      case MethodType::PUT: {
        size_t index1 = call.arg.find_first_of('-');
        size_t index2 = call.arg.find_last_of('-');
        std::string key = call.arg.substr(0, index1);
        std::string val = call.arg.substr(index1 + 1, index2 - index1 - 1);
        std::string ts = call.arg.substr(index2 + 1, call.arg.length());
        putDownstream(std::stoi(key), std::stoi(val), std::stoi(ts));
        break;
      }
      case MethodType::GET:
        return this;
        break;
      default:
        std::cout << "wrong method name" << std::endl;
        break;
    }
    return this;
  }

  virtual void toString() {
    std::cout << "keysvalues: " << keysvalues.size() << std::endl;
  }

  virtual bool isPermissible(MethodCall call) { return true; }
};