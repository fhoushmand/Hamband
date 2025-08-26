#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>

#include "../src/replicated_object.hpp"

typedef unsigned char uint8_t;

class KvStore : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      PUT = 0,
      GET = 1
    };

    std::vector<std::atomic<int>> keysvalues (1000000);
    KvStore()
    {
      read_methods.push_back(static_cast<int>(MethodType::GET));
      
      update_methods.push_back(static_cast<int>(MethodType::PUT));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::PUT), 2));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::GET), 0));
      
      // conflicts
      std::vector<int> g1;
      g1.push_back(static_cast<int>(MethodType::PUT));
      synch_groups.push_back(g1);
      
      // dependencies
      //std::vector<int> d1;
      //d1.push_back(static_cast<int>(MethodType::DEPOSIT));
      //dependency_relation.insert(std::make_pair(static_cast<int>(MethodType::PUT), d1));
    }

     // Courseware(const Courseware&) = delete;
    KvStore(KvStore &obj) : ReplicatedObject(obj)
    {
      //state
      this->keysvalues = obj.keysvalues.load();
    }

    virtual void toString()
    {
      std::cout << "keys values size: " << keysvalues.size() << std::endl;
    }

    ~KvStore(){}

    void put(std::string key, std::string value)
    {
      int k = std::stoi(key);
      int v = std::stoi(value);
      keysvalues[k].store(value, std::memory_order_relaxed);
    }
    KvStore get() { return *this;}

    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::PUT:
      {
        size_t index = call.arg.find_first_of('-');
        std::string key = call.arg.substr(0, index);
        std::string value = call.arg.substr(index + 1, call.arg.length());
        put(key, value);
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

    virtual bool isPermissible(MethodCall call)
    {
      return true;
    }
};