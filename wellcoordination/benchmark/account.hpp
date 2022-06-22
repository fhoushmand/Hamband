#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>

#include "../src/replicated_object.hpp"

typedef unsigned char uint8_t;

class BankAccount : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      WITHDRAW = 0,
      DEPOSIT = 1,
      QUERY = 2
    };

    std::atomic<int> balance;
    BankAccount(int b)
    {
      balance = b;

      read_methods.push_back(static_cast<int>(MethodType::QUERY));
      
      update_methods.push_back(static_cast<int>(MethodType::WITHDRAW));
      update_methods.push_back(static_cast<int>(MethodType::DEPOSIT));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::WITHDRAW), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::DEPOSIT), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));
      
      // conflicts
      std::vector<int> g1;
      g1.push_back(static_cast<int>(MethodType::WITHDRAW));
      synch_groups.push_back(g1);
      
      // dependencies
      std::vector<int> d1;
      d1.push_back(static_cast<int>(MethodType::DEPOSIT));
      dependency_relation.insert(std::make_pair(static_cast<int>(MethodType::WITHDRAW), d1));
    }

     // Courseware(const Courseware&) = delete;
    BankAccount(BankAccount &obj) : ReplicatedObject(obj)
    {
      //state
      this->balance = obj.balance.load();
    }

    virtual void toString()
    {
      std::cout << "balance: " << balance << std::endl;
    }

    ~BankAccount(){}

    void withdraw(int a) { balance -= a;}
    void deposit(int a) { balance += a;}
    BankAccount query() { return *this;}

    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::DEPOSIT:
      {
        int aa = std::stoi(call.arg);
        // std::cout << "executing deposit " << aa << std::endl;
        deposit(aa);
        break;
      }
      case MethodType::WITHDRAW:
      {
        int a = std::stoi(call.arg);
        // std::cout << "executing withdraw " << a << std::endl;
        withdraw(a);
        break;
      }
      case MethodType::QUERY:
        return this;
        break;
      default:
        std::cout << "wrong method name" << std::endl;
        break;
      }
      return this;
    }

    virtual bool isPermissible(MethodCall call)
    {
      MethodType method_type = static_cast<MethodType>(call.method_type);
      if(method_type == DEPOSIT || method_type == QUERY)
        return true;
      else
      {
        int a = std::stoi(call.arg);
        if(a > balance)
          return false;
        return true;
      }
      return false;
    }
};