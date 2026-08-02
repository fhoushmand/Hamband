
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

class PNSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };
    std::atomic<bool> lock;
    int pnset[200001]={0};
    //int arraysize=0;
    //int arraysize=0;
    //int setsizesource=0;
    int setsize=0;
    //std::set<std::string> addset;
    //std::set<std::string> removeset;
    
    
 
    PNSet() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REMOVE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));
    }

    PNSet(PNSet &obj) : ReplicatedObject(obj)
    {
      //state
      std::memcpy(pnset, obj.pnset, sizeof(pnset));
      //arraysize = obj.arraysize;
      setsize= obj.setsize;
      //removeset = obj.removeset;
    }

    virtual void toString()
    {
      std::cout << "#elements: " << (setsize) << std::endl;
    }

    
    // 0
    void add(std::string a)
    {

      //bool find=false;
      while(lock.load());
      lock.store(true);
      pnset[std::stoi(a)]++;
      setsize++;
      lock.store(false);
    }
    void remove(std::string a)
    {
      //bool find=false;
      while(lock.load());
      lock.store(true);
      //find=false;
      pnset[std::stoi(a)]--;
      setsize--;
      lock.store(false);
    }
    // 1
    PNSet query() { return *this; }


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