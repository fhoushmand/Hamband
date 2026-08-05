#include <atomic>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../src/replicated_object.hpp"
#include "state_digest.hpp"

typedef unsigned char uint8_t;

class SmallBank : public ReplicatedObject {
 private:
 public:
  enum MethodType { WITHDRAW = 0, DEPOSIT = 1, QUERY = 2 };

  size_t account_number;
  std::shared_ptr<std::atomic<int>[]> accounts;

  SmallBank(int b, int account_number_) {
    account_number = static_cast<size_t>(account_number_);

    accounts = std::shared_ptr<std::atomic<int>[]>(
        new std::atomic<int>[account_number],
        std::default_delete<std::atomic<int>[]>());

    // initialize all accounts with balance b
    for (size_t i = 0; i < account_number; i++) accounts[i].store(b);

    read_methods.push_back(static_cast<int>(MethodType::QUERY));

    update_methods.push_back(static_cast<int>(MethodType::WITHDRAW));
    update_methods.push_back(static_cast<int>(MethodType::DEPOSIT));

    method_args.insert(
        std::make_pair(static_cast<int>(MethodType::WITHDRAW), 2));
    method_args.insert(
        std::make_pair(static_cast<int>(MethodType::DEPOSIT), 2));
    method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 1));

    // conflicts
    std::vector<int> g1;
    g1.push_back(static_cast<int>(MethodType::WITHDRAW));
    synch_groups.push_back(g1);

    // dependencies
    std::vector<int> d1;
    d1.push_back(static_cast<int>(MethodType::DEPOSIT));
    dependency_relation.insert(
        std::make_pair(static_cast<int>(MethodType::WITHDRAW), d1));
  }

  SmallBank(SmallBank& obj) : ReplicatedObject(obj) {
    this->account_number = obj.account_number;
    this->accounts = obj.accounts;  // share the same array
  }

  virtual void toString() {
    uint64_t digest = 0;
    int64_t total_balance = 0;
    for (size_t account = 0; account < account_number; account++) {
      int balance = accounts[account].load();
      total_balance += balance;
      state_digest::addOrdered(
          digest, static_cast<uint64_t>(static_cast<int64_t>(balance)));
    }
    std::cout << "number of accounts: " << account_number << std::endl;
    std::cout << "total balance: " << total_balance << std::endl;
    std::cout << "state digest: " << digest << std::endl;
  }

  ~SmallBank() {}

  void withdraw(int aid, int v) { accounts[aid] -= v; }
  void deposit(int aid, int v) { accounts[aid] += v; }
  int query(int aid) { return accounts[aid].load(); }

  virtual ReplicatedObject* execute(MethodCall call) {
    switch (static_cast<MethodType>(call.method_type)) {
      case MethodType::DEPOSIT: {
        size_t index = call.arg.find_first_of('-');
        std::string aid = call.arg.substr(0, index);
        std::string v = call.arg.substr(index + 1, call.arg.length());
        deposit(std::stoi(aid), std::stoi(v));
        break;
      }
      case MethodType::WITHDRAW: {
        size_t index = call.arg.find_first_of('-');
        std::string aid = call.arg.substr(0, index);
        std::string v = call.arg.substr(index + 1, call.arg.length());
        withdraw(std::stoi(aid), std::stoi(v));
        break;
      }
      case MethodType::QUERY:
        query(std::stoi(call.arg));
        break;
      default:
        std::cout << "wrong method name" << std::endl;
        break;
    }
    return this;
  }

  virtual bool isPermissible(MethodCall call) {
    MethodType method_type = static_cast<MethodType>(call.method_type);
    if (method_type == DEPOSIT || method_type == QUERY)
      return true;
    else {
      size_t index = call.arg.find_first_of('-');
      std::string aid = call.arg.substr(0, index);
      std::string v = call.arg.substr(index + 1, call.arg.length());
      if (std::stoi(v) > accounts[std::stoi(aid)].load()) return false;
      return true;
    }
    return false;
  }
};
