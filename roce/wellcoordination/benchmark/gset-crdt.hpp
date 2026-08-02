
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

class GSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      QUERY = 1
    };
    std::recursive_mutex m;
    std::set<std::string> setsource;
    std::set<std::string> setremote;
    
    
 
    GSet() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD));
      
      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));
    }

    GSet(GSet &obj) : ReplicatedObject(obj)
    {
      //state
      setsource = obj.setsource;
      setsource = obj.setremote;
    }

    virtual void toString()
    {
      std::cout << "#elementssource: " << setsource.size() << std::endl;
      std::cout << "#elementsremote: " << setremote.size() << std::endl;
    }


    // 0
    std::string add(std::string a)
    {
      return "";
    }
    std::string addDownstream(std::string a, bool b)
    {
      //std::scoped_lock lock(m);
      if (b==false)
        setsource.insert(a);
      else
        setremote.insert(a);
      return "";
    }
    // 1
    GSet query() { return *this; }


    virtual std::string execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
        return add(call.arg);
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