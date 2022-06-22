
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_set>
#include <mutex>

#include "../src/replicated_object_crdt.hpp"


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
    std::string add(int a)
    {
      return "";
    }
    // 1
    Counter query() { return *this; }


    //downstream
    void addDownstream(int a)
    {
      counter += a;
    }


    virtual std::string execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
        return add(std::stoi(call.arg));
        break;
      case MethodType::QUERY:
        return "";
        break;
      default:
        std::cout << "wrong method name" << std::endl;
        break;
      }
      return "";
    }


    virtual ReplicatedObject* executeDownstream(MethodCall call, bool b)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
      {
        //std::cout << "arg: " << call.arg << std::endl;
        size_t index = call.arg.find_first_of('-');
        if(index != std::string::npos){
          std::string arg = call.arg.substr(0, index);
          addDownstream(std::stoi(arg));
        }
        else
          addDownstream(std::stoi(call.arg));

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
        return true;
    }
};