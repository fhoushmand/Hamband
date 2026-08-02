
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <set>
// #include <unordered_map>
#include "../src/cuckoohash_map.hh"
#include <mutex>

#include "../src/replicated_object_crdt.hpp"


typedef unsigned char uint8_t;

class ORSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };

    std::recursive_mutex m;
    // std::unordered_map<std::string, std::set<std::string>> local_set;
    // std::unordered_map<std::string, std::set<std::string>> remote_set;
    libcuckoo::cuckoohash_map<std::string, std::set<std::string>> set;
    
    int numAdds;
 
    ORSet() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REMOVE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));
    }


    ORSet(ORSet &obj) : ReplicatedObject(obj)
    {
      //state
      this->set = obj.set;
      // this->local_set = obj.local_set;
      // this->remote_set = obj.remote_set;
      this->numAdds = obj.numAdds;
    }

    virtual void toString()
    {
      // std::scoped_lock lock(m);
      // std::cout << "#elements: " << local_set.size() + remote_set.size() << std::endl;
      std::cout << "#elements: " << set.size() << std::endl;
    }
   
    // 0
    std::string add(std::string e)
    {
      return std::to_string(self) + std::to_string(++numAdds);
    }

    void addDownstream(std::string e, std::string tag, bool local)
    {

      auto updateTags = [&](std::set<std::string> &tags) { tags.insert(tag); };
      std::set<std::string> tags;
      tags.insert(tag);
      set.upsert(e, updateTags, tags);
      // std::cout << "add downstream" << std::endl;

      // auto lt = set.lock_table();
      // if(lt.find(e) == lt.end())
      //   {
      //     // std::cout << "if" << std::endl;
      //     std::set<std::string> tags;
      //     // std::cout << "if1" << std::endl;
      //     tags.insert(tag);
      //     // set2.insert(e,10);
      //     // std::cout << "if2" << std::endl;
      //     lt.insert(e, tags);
      //     // std::cout << "if3" << std::endl;
      //   }
      //   else{
      //     // std::cout << "else" << std::endl;
      //     lt.at(e).insert(tag);
      //   }
    }


    std::string remove(std::string e) 
    {
      std::string toRemoveString = "";
      auto lt = set.lock_table();
      for(std::string t : lt.at(e))
        toRemoveString += t + ",";
      return toRemoveString;
      
    }

    void removeDownstream(std::string e, std::string removeSet, bool local)
    {
      size_t pos = 0;  
      std::set<std::string> orderedRemoveSet;
      // std::unordered_map<std::string, std::set<std::string>> set = local ? local_set : remote_set;
      while (( pos = removeSet.find (',')) != std::string::npos)
      { 
        // std::cout << removeSet << std::endl; 
        std::string toRemoveTag = removeSet.substr(0, pos);   
        removeSet.erase(0, pos + 1);
        orderedRemoveSet.insert(toRemoveTag);
        
      }
      bool delivered = false;  
      auto lt = set.lock_table();
          for (auto& p : lt) {
            if(p.first == e){
              if(p.second == orderedRemoveSet){
                delivered = true;
                break;
              }
            }
          }
          if(!delivered)
            return;
        lt.erase(e);
    }

    // 1
    ORSet query() { return *this; }


    virtual std::string execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
      {
        return add(call.arg);
        break;
      }
      case MethodType::REMOVE:
      {
        return remove(call.arg);
        break;
      }
      case MethodType::QUERY:
        return "";
        break;
      default:
        std::cout << "wrong method name" << std::endl;
        break;
      }
      return "";
    }

    virtual ReplicatedObject* executeDownstream(MethodCall call, bool local)
    {
      // std::cout << "method: " << static_cast<int>(call.method_type) << "\t" <<  call.arg << std::endl;
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
      {
        size_t index = call.arg.find_first_of('-');
        std::string e = call.arg.substr(0, index);
        std::string tag = call.arg.substr(index + 1, call.arg.length());
        addDownstream(e, tag, local);
        break;
      }
      case MethodType::REMOVE:
      {
        size_t index = call.arg.find_first_of('-');
        if(index != std::string::npos)
        {
          std::string e = call.arg.substr(0, index);
          std::string remSet = call.arg.substr(index + 1, call.arg.length());
          removeDownstream(e, remSet, local);
        }
        else
          removeDownstream(call.arg.substr(0, call.arg.length() - 1), "", local);
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