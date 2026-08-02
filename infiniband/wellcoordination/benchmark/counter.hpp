
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_set>
#include <mutex>

#include "../src/replicated_object.hpp"


typedef unsigned char uint8_t;

class Counter : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      QUERY = 1
    };

    std::atomic<int> counter;
    
    
 
    Counter() {
      counter = 0;
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));
    }

    Counter(Counter &obj) : ReplicatedObject(obj)
    {
      //state
      this->counter = obj.counter.load();
    }

    virtual void toString()
    {
      std::cout << "counter: " << counter << std::endl;
    }

   
    // 0
    void add(int a)
    {
      counter += a;
    }
    // 1
    Counter query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
        add(std::stoi(call.arg));
        break;
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
        return true;
    }
};