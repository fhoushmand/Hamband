
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_set>
#include <mutex>

#include "replicated_object.hpp"


typedef unsigned char uint8_t;

class Cart : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };

    std::set<std::string> set;
    
    
 
    Cart() {
      num_methods = 3;
      read_method = 2;
      update_methods.push_back(static_cast<int>(MethodType::ADD));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE));
    }

    Cart(Cart &obj) : ReplicatedObject(obj)
    {
      //state
      set = obj.set;
    }

    virtual void toString()
    {
      std::cout << "#elements: " << set.size() << std::endl;
    }

   
    // 0
    void add(std::string a)
    {
      set.insert(a);
    }
    // 1
    void remove(std::string a)
    {
      set.erase(a);
    }
    // 2
    Cart query() { return *this; }


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