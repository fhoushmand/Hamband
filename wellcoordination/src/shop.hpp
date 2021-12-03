
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_map>
#include <mutex>

#include "replicated_object.hpp"


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
    
    
 
    Shop() {
      num_methods = 3;
      read_method = 2;
      update_methods.push_back(static_cast<int>(MethodType::ADD));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE));
    }

    Shop(Shop &obj) : ReplicatedObject(obj)
    {
      //state
      multiset = obj.multiset;
    }

    virtual void toString()
    {
      std::cout << "#elements: " << multiset.size() << std::endl;
    }

   
    // 0
    void add(int a)
    {
      if(multiset.find(a) != multiset.end())
        multiset.find(a)->second++;
      else
        multiset.insert(std::make_pair(a, 1));
    }
    // 1
    void remove(int a)
    {
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
        add(std::stoi(call.arg));
        break;
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
        return true;
    }
};