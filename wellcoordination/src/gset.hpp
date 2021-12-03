
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

class GSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      QUERY = 1
    };

    std::set<std::string> set;
    
    
 
    GSet() {
      num_methods = 2;
      read_method = 1;
      update_methods.push_back(static_cast<int>(MethodType::ADD));
    }

    GSet(GSet &obj) : ReplicatedObject(obj)
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
    GSet query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
        add(call.arg);
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