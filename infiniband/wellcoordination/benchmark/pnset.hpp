
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

class PNSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };
    std::mutex state_mutex;
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
      uint64_t digest = 0;
      for (size_t index = 0; index < 200001; index++) {
        state_digest::addOrdered(
            digest, static_cast<uint64_t>(static_cast<int64_t>(pnset[index])));
      }
      std::cout << "#elements: " << (setsize) << std::endl;
      std::cout << "state_digest: " << digest << std::endl;
    }

    
    // 0
    void add(std::string a)
    {

      //bool find=false;
      const std::lock_guard<std::mutex> lock(state_mutex);
      pnset[std::stoi(a)]++;
      setsize++;
    }
    void remove(std::string a)
    {
      //bool find=false;
      const std::lock_guard<std::mutex> lock(state_mutex);
      //find=false;
      pnset[std::stoi(a)]--;
      setsize--;
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
