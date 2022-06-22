
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <set>
#include <unordered_map>
#include <junction/ConcurrentMap_Leapfrog.h>
#include <junction/ConcurrentMap_Grampa.h>
#include <turf/RWLock.h>
#include <turf/Mutex.h>

#include "../src/replicated_object_crdt.hpp"


typedef unsigned char uint8_t;
typedef junction::ConcurrentMap_Leapfrog<std::string, int> ConcurrentSet;
typedef junction::ConcurrentMap_Grampa<int, ConcurrentSet*> ConcurrentMap;

class ORSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };

    turf::Mutex m;
    turf::RWLock m_rwLock;
    ConcurrentMap set;
    
    long long numAdds = 0;
 
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
      this->numAdds = obj.numAdds;
    }

    static void destroy(std::set<std::string>* set)
    {
      delete set;
    }

    virtual void toString()
    {
      // std::scoped_lock lock(m);
      // std::cout << "#elements: " << local_set.size() + remote_set.size() << std::endl;
      // std::cout << "#elements: " << set.size() << std::endl;
    }
   
    // 0
    std::string add(std::string e)
    {
      std::string tag = std::to_string(self) + std::to_string(++numAdds);
      return tag;
    }

    void addDownstream(std::string e, std::string tag, bool local)
    {
      ConcurrentMap::Mutator mutator = set.insertOrFind(std::stoi(e));
      ConcurrentSet* value = mutator.getValue();
      if (!value) {
          value = new ConcurrentSet();
          value->assign(tag, 1);
          mutator.assignValue(value);
      }
      else
      {
        turf::ExclusiveLockGuard<turf::RWLock> guard(m_rwLock);
        value->assign(tag, 1);
      }
    }


    std::string remove(std::string e) 
    {
      if(!set.get(std::stoi(e))) return "";
      std::string toRemoveString = "";

      turf::ExclusiveLockGuard<turf::RWLock> guard(m_rwLock);
      for (ConcurrentSet::Iterator iter(*set.get(std::stoi(e))); iter.isValid(); iter.next()) {
      // for(std::string t : *set.get(std::stoi(e)))
        toRemoveString += iter.getKey() + ",";
      }
      return toRemoveString;
    }

    bool isFirstEqualSecond(std::set<std::string> first, std::set<std::string> second) {
      if(first.size() > second.size())
          return false;
      std::vector<std::string> out(first.size());
      std::set_intersection(first.begin(), first.end(), second.begin(), second.end(), out.begin());
      
      size_t i = 0;
      for(std::string s : first){
        if(out.at(i++) != s) return false;
      }
      return true;
    }

    bool isFirstEqualSecond(std::set<std::string> first, ConcurrentSet set) {
      std::set<std::string> second;
      for (ConcurrentSet::Iterator iter(set`); iter.isValid(); iter.next()) {
        second.insert(iter.getKey());
      }
      if(first.size() > second.size())
          return false;
      std::vector<std::string> out(first.size());
      std::set_intersection(first.begin(), first.end(), second.begin(), second.end(), out.begin());
      
      size_t i = 0;
      for(std::string s : first){
        if(out.at(i++) != s) return false;
      }
      return true;
    }

    void removeDownstream(std::string e, std::string removeSet, bool local)
    {
      size_t pos = 0;  
      std::set<std::string> orderedRemoveSet;
      while (( pos = removeSet.find (',')) != std::string::npos)
      { 
        std::string toRemoveTag = removeSet.substr(0, pos);   
        removeSet.erase(0, pos + 1);
        orderedRemoveSet.insert(toRemoveTag);
      }

      turf::ExclusiveLockGuard<turf::RWLock> guard(m_rwLock);
      if(set.get(std::stoi(e))){
        // the remove set is bigger than what the node aleady received, returning...
        if(isFirstEqualSecond(orderedRemoveSet, *set.get(std::stoi(e))))
          set.erase(std::stoi(e));
      }
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
