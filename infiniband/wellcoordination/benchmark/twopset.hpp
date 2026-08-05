
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_set>
#include <mutex>

#include "../src/replicated_object.hpp"
#include "state_digest.hpp"


typedef unsigned char uint8_t;

class TWOPSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };
    std::mutex add_mutex;
    std::mutex remove_mutex;
    std::set<std::string> addset;
    std::set<std::string> removeset;
    
    
 
    TWOPSet() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REMOVE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));
    }

    TWOPSet(TWOPSet &obj) : ReplicatedObject(obj)
    {
      //state
      addset = obj.addset;
      removeset = obj.removeset;
    }

    virtual void toString()
    {
      size_t elements = 0;
      uint64_t digest = 0;
      for (auto const& value : addset) {
        if (removeset.count(value) == 0) {
          elements++;
          state_digest::addUnordered(digest, state_digest::string(value));
        }
      }
      std::cout << "#elements: " << elements << std::endl;
      std::cout << "state_digest: " << digest << std::endl;
    }

    
    // 0
    void add(std::string a)
    {
      const std::lock_guard<std::mutex> lock(add_mutex);
      addset.insert(a);
    }
    void remove(std::string a)
    {
      const std::lock_guard<std::mutex> lock(remove_mutex);
      removeset.insert(a);
    }
    // 1
    TWOPSet query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
        add(call.arg);
        break;
      case MethodType::REMOVE:
        remove(call.arg);
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
