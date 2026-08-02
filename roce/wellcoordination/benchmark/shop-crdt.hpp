
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <junction/ConcurrentMap_Leapfrog.h>
#include <junction/ConcurrentMap_Grampa.h>
#include <turf/RWLock.h>
#include <turf/Mutex.h>

#include "../src/replicated_object_crdt.hpp"


//#include <cds/intrusive/lazy_list_nogc.h>
            // Declare item stored in your list
            struct TagNumber //: public cds::intrusive::lazy_list::node< cds::gc::nogc >
            {
                std::string     tag    ; 
                std::string     number  ;  
                // ...  other value fields

                TagNumber(std::string t, std::string n){
                  this->tag = t;
                  this->number = n;
                }

                TagNumber(const TagNumber& tag)
                {
                  this->tag = tag.tag;
                  this->number = tag.number;
                }

                friend bool operator==(const TagNumber& lhs, const TagNumber& rhs)
                {
                    return lhs.tag.compare(rhs.tag) == 0 && lhs.number.compare(rhs.number) == 0;
                }
            };

            // Declare comparator for the item
            struct my_compare_pair {
                int operator()( TagNumber const& i1, TagNumber const& i2 ) const
                {
                    int tagCompare = i1.tag.compare(i2.tag);
                    return (!tagCompare ? i1.number.compare(i2.number) : tagCompare);
                }
                // int operator()( const TagNumber& d, const std::string& s )
                // {
                //     int tagCompare = d.tag.compare(s);
                //     return (!tagCompare ? d.number.compare(i2.number) : tagCompare);

                //     return d.tag.compare(s);
                // }
                // int operator()( const std::string& s, const TagNumber& d )
                // {
                //     return s.compare( d.tag );
                // }
            };

            // Declare traits
            // struct my_traits: public cds::intrusive::lazy_list::traits
            // {
            //     typedef cds::intrusive::lazy_list::base_hook< cds::opt::gc< cds::gc::nogc > >   hook;
            //     typedef my_compare_pair compare;
            // };

            // typedef cds::intrusive::LazyList< cds::gc::nogc, TagNumber, my_traits > my_set_pair;
            typedef std::set<TagNumber,my_compare_pair> my_set_pair;

            


typedef unsigned char uint8_t;
typedef junction::ConcurrentMap_Grampa<int, my_set_pair*> ConcurrentMapPair;

class Shop : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };

    turf::Mutex m;
    turf::Mutex m_rwLock;
    ConcurrentMapPair set;
    
    long long numAdds = 0;
 
    Shop() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REMOVE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));
    }


    Shop(Shop &obj) : ReplicatedObject(obj)
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
       long long iterCount = 0;
      for (ConcurrentMapPair::Iterator iter(set); iter.isValid(); iter.next()) {
        // std::cout << "key: " << iter.getKey() << std::endl;
        iterCount++;
      }
      std::cout << "size: " << iterCount << std::endl;
    }



    // pre condition: first \subsetequal second
    // output: second - first
    // first: toBeRemoved
    // second: already received and in the set
    std::pair<bool,my_set_pair> setMinusWithGuard(my_set_pair first, my_set_pair* second) {
      if(first.size() > second->size())
        return std::make_pair(false, *second);

      my_set_pair intersection; // second /\ first
      my_set_pair difference; // second - first
      my_set_pair::iterator first1 = first.begin();
      my_set_pair::iterator last1 = first.end();
      my_set_pair::iterator first2 = second->begin();
      my_set_pair::iterator last2 = second->end();
       
      while (first1!=last1 && first2!=last2)
      {
        int compare = my_compare_pair()(*first1, *first2);
        if (compare < 0) { ++first1; }
        else if (compare > 0) { difference.insert(*first2); ++first2; }
        else {
          intersection.insert(*first1);
          ++first1; ++first2;
        }
      }
      if(intersection == first)
        return std::make_pair(true, difference);
      else
        return std::make_pair(false, *second);
    }

   
    // 0
    std::string add(std::string e, std::string n)
    {
      std::string tag = std::to_string(self) + std::to_string(++numAdds);

      if(!set.get(std::stoi(e))) return "";
      std::string toRemoveString = "";
      {
        // turf::LockGuard<turf::Mutex> guard(m_rwLock);
        for(TagNumber t : *set.get(std::stoi(e)))
          toRemoveString += t.tag + ":" + t.number + ",";
      }
      return tag + "-" + n + "-" + toRemoveString;
    }

    void addDownstream(std::string e, std::string tag, std::string n, std::string removeSet)
    {
      size_t pos = 0;  
      my_set_pair orderedRemoveTags;
      while (( pos = removeSet.find (',')) != std::string::npos)
      { 
        std::string tagNumber = removeSet.substr(0, pos);   
        size_t index = tagNumber.find_first_of('-');
        std::string tag = tagNumber.substr(0, index);
        std::string number = tagNumber.substr(index + 1, tagNumber.length());

        removeSet.erase(0, pos + 1);
        orderedRemoveTags.insert(TagNumber(tag,number));
      }
      
      my_set_pair* value = set.get(std::stoi(e));
      if(value){
        std::pair<bool,my_set_pair> result = setMinusWithGuard(orderedRemoveTags, value);
        if(result.first)
        {
          my_set_pair* value = set.get(std::stoi(e));
          my_set_pair* new_value = new my_set_pair();
          new_value->insert(TagNumber(tag, n));
            for(TagNumber t : *value)
              new_value->insert(t);
          set.assign(std::stoi(e), new_value);
        }
      }
      else {
        my_set_pair* new_value = new my_set_pair();
        new_value->insert(TagNumber(tag,n));
        set.assign(std::stoi(e), new_value);
      }

    }


    std::string remove(std::string e) 
    {
      if(!set.get(std::stoi(e))) return "";
      std::string toRemoveString = "";
      {
        for(TagNumber t : *set.get(std::stoi(e)))
          toRemoveString += t.tag + ":" + t.number + ",";
      }
      return toRemoveString;
    }


    void removeDownstream(std::string e, std::string removeSet)
    {
      size_t pos = 0;  
      my_set_pair orderedRemoveTags;
      while (( pos = removeSet.find (',')) != std::string::npos)
      { 
        std::string tagNumber = removeSet.substr(0, pos);   
        size_t index = tagNumber.find_first_of('-');
        std::string tag = tagNumber.substr(0, index);
        std::string number = tagNumber.substr(index + 1, tagNumber.length());

        removeSet.erase(0, pos + 1);
        orderedRemoveTags.insert(TagNumber(tag,number));
      }

      my_set_pair* value = set.get(std::stoi(e));
      
      if(value){
        // the remove set is bigger than what the node aleady received, returning...
        std::pair<bool,my_set_pair> result = setMinusWithGuard(orderedRemoveTags, value);
        if(result.first)
        {
          my_set_pair* new_value = new my_set_pair();
          for(TagNumber tn : result.second)
            new_value->insert(tn);
          
          set.assign(std::stoi(e), new_value);
        }
      }
    }

    // 1
    Shop query() { return *this; }


    virtual std::string execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
      {
        // std::string* args = ReplicatedObject::parseArgs(call.arg, 2);
        // return add(args[0], args[1]);
        size_t index = call.arg.find_first_of('-');
        std::string e = call.arg.substr(0, index);
        std::string n = call.arg.substr(index + 1, call.arg.length());
        add(e, n);
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

    virtual ReplicatedObject* executeDownstream(MethodCall call, bool b)
    {
      // std::cout << "method: " << static_cast<int>(call.method_type) << "\t" <<  call.arg << std::endl;
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
      {
        // std::string* args = ReplicatedObject::parseArgs(call.arg, 4);
        // addDownstream(args[0], args[1], args[2], args[3]);
        size_t index = call.arg.find_first_of('-');
        std::string e = call.arg.substr(0, index);
        std::string tagNSet = call.arg.substr(index + 1, call.arg.length());
        size_t index2 = tagNSet.find_first_of('-');
        std::string tag = tagNSet.substr(0, index2);
        std::string nSet = tagNSet.substr(index2 + 1, tagNSet.length());
        size_t index3 = nSet.find_first_of('-');
        std::string n = nSet.substr(0, index3);
        std::string set = nSet.substr(index3 + 1, nSet.length());
        addDownstream(e, tag, n, set);
        break;
      }
      case MethodType::REMOVE:
      {
        size_t index = call.arg.find_first_of('-');
        if(index != std::string::npos)
        {
          std::string e = call.arg.substr(0, index);
          std::string remSet = call.arg.substr(index + 1, call.arg.length());
          removeDownstream(e, remSet);
        }
        else
          removeDownstream(call.arg.substr(0, call.arg.length() - 1), "");
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
