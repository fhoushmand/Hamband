
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

class TWOPSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };
    std::recursive_mutex m;
    std::set<std::string> setsourceadd;
    std::set<std::string> setremoteadd;
    std::set<std::string> setsourceremove;
    std::set<std::string> setremoteremove;
    
    
 
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
      setsourceadd = obj.setsourceadd;
      setsourceremove = obj.setsourceremove;
      setremoteadd = obj.setremoteadd;
      setremoteremove = obj.setremoteremove;
    }

    virtual void toString()
    {
      std::cout << "#elementssource: " << (setsourceadd.size()-setsourceremove.size()) << std::endl;
      std::cout << "#elementsremote: " << (setremoteadd.size()-setremoteremove.size()) << std::endl;
    }


    // 0
    std::string add(std::string a)
    {
      return "";
    }
    std::string remove(std::string a)
    {
      return "";
    }
    std::string addDownstream(std::string a, bool b)
    {
      //std::scoped_lock lock(m);
      if (b==false)
        setsourceadd.insert(a);
      else
        setremoteadd.insert(a);
      return "";
    }
    std::string removeDownstream(std::string a, bool b)
    {
      //std::scoped_lock lock(m);
      if (b==false)
        setsourceremove.insert(a);
      else
        setremoteremove.insert(a);
      return "";
    }
    // 1
    TWOPSet query() { return *this; }


    virtual std::string execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
        return add(call.arg);
        break;
      case MethodType::REMOVE:
        return remove(call.arg);
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
        size_t index = call.arg.find_first_of('-');
        if(index != std::string::npos){
          std::string arg = call.arg.substr(0, index);
          addDownstream((arg), b);
        }
        else
          addDownstream((call.arg), b);

        break;
      }
      case MethodType::REMOVE:
      {
        size_t index = call.arg.find_first_of('-');
        if(index != std::string::npos){
          std::string arg = call.arg.substr(0, index);
          removeDownstream((arg), b);
        }
        else
          removeDownstream((call.arg), b);

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