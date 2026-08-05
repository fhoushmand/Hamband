
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_map>
#include <mutex>

#include "../src/replicated_object.hpp"
#include "state_digest.hpp"


typedef unsigned char uint8_t;

class Shop : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };

    std::unordered_map<int,int> multiset;
    std::mutex multiset_mutex;
    
 
    Shop() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REMOVE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));

      std::vector<int> updates;
      updates.push_back(static_cast<int>(MethodType::REMOVE));
      synch_groups.push_back(updates);

      std::vector<int> dependencies;
      dependencies.push_back(static_cast<int>(MethodType::ADD));
      dependency_relation.insert(
          std::make_pair(static_cast<int>(MethodType::REMOVE), dependencies));
    }

    Shop(Shop &obj) : ReplicatedObject(obj)
    {
      //state
      multiset = obj.multiset;
    }

    virtual void toString()
    {
      const std::lock_guard<std::mutex> lock(multiset_mutex);
      uint64_t digest = 0;
      for (auto const& entry : multiset) {
        state_digest::addUnordered(
            digest, state_digest::mix(static_cast<uint64_t>(entry.first)) ^
                        static_cast<uint64_t>(entry.second));
      }
      std::cout << "#elements: " << multiset.size() << std::endl;
      std::cout << "state_digest: " << digest << std::endl;
    }

   
    // 0
    void add(int a, int quantity)
    {
      const std::lock_guard<std::mutex> lock(multiset_mutex);
      if(multiset.find(a) != multiset.end())
        multiset.find(a)->second += quantity;
      else
        multiset.insert(std::make_pair(a, quantity));
    }
    // 1
    void remove(int a)
    {
      const std::lock_guard<std::mutex> lock(multiset_mutex);
      if(multiset.find(a) != multiset.end()){
        if(multiset.find(a)->second == 1)
          multiset.erase(a);
        else
          multiset.find(a)->second--;
      }
    }
    // 2
    Shop query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
      {
        size_t index = call.arg.find_first_of('-');
        int item = std::stoi(call.arg.substr(0, index));
        int quantity = index == std::string::npos
                           ? 1
                           : std::stoi(call.arg.substr(index + 1));
        add(item, quantity);
        break;
      }
      case MethodType::REMOVE:
        remove(std::stoi(call.arg));
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
      MethodType method = static_cast<MethodType>(call.method_type);
      if (method != MethodType::REMOVE) {
        return true;
      }
      int item = std::stoi(call.arg);
      const std::lock_guard<std::mutex> lock(multiset_mutex);
      auto entry = multiset.find(item);
      return entry != multiset.end() && entry->second > 0;
    }
};
