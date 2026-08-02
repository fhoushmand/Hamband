
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


// // Data stored in cuckoo set
// struct my_data
// {
//     // key field
//     std::string     strKey;
//     // other data
//     // ...
// };
// // Provide compare functor for my_data since we want to use ordered probe-set
// struct my_data_compare {
//     int operator()( const my_data& d1, const my_data& d2 ) const
//     {
//         return d1.strKey.compare( d2.strKey );
//     }
//     int operator()( const my_data& d, const std::string& s ) const
//     {
//         return d.strKey.compare(s);
//     }
//     int operator()( const std::string& s, const my_data& d ) const
//     {
//         return s.compare( d.strKey );
//     }
// };
// // Provide two hash functor for my_data
// struct hash1 {
//     size_t operator()(std::string const& s) const
//     {
//         return std::hash<std::string>{}(s);
//     }
//     size_t operator()( my_data const& d ) const
//     {
//         return (*this)( d.strKey );
//     }
// };
// struct hash2: private hash1 {
//     size_t operator()(std::string const& s) const
//     {
//         size_t h = ~( hash1::operator()(s));
//         return ~h + 0x9e3779b9 + (h << 6) + (h >> 2);
//     }
//     size_t operator()( my_data const& d ) const
//     {
//         return (*this)( d.strKey );
//     }
// };
// // Declare type traits
// // We use a vector of capacity 4 as probe-set container and store hash values in the node
// // struct my_traits: public cds::container::cuckoo::traits
// // {
// //     typedef my_data_compare compare;
// //     typedef std::tuple< hash1, hash2 >  hash;
// //     typedef cds::container::cuckoo::vector<4> probeset_type;
// //     static bool const store_hash = true;
// // };
// // Declare CuckooSet type
// // typedef cds::container::CuckooSet< my_data, my_traits > my_cuckoo_set;
// // Equal option-based declaration
// typedef cds::container::CuckooSet< my_data,
//     cds::container::cuckoo::make_traits<
//         cds::opt::hash< std::tuple< hash1, hash2 > >
//         ,cds::opt::compare< my_data_compare >
//         ,cds::container::cuckoo::probeset_type< cds::container::cuckoo::vector<4> >
//         ,cds::container::cuckoo::store_hash< true >
//     >::type
// > my_cuckoo_set;


// // For gc::HP-based MichaelList implementation
// #include <cds/intrusive/michael_list_hp.h>
// // cds::intrusive::MichaelHashSet declaration
// #include <cds/intrusive/michael_set.h>
// // Type of hash-set items
// struct Tag: public cds::intrusive::michael_list::node< cds::gc::HP >
// {
//     std::string     key_    ;   // key field
//     // ...  other value fields

//     Tag(std::string t){
//       this->key_ = t;
//     }
// };
// // Declare comparator for the item
// struct TagCmp
// {
//     int operator()( const Tag& f1, const Tag& f2 ) const
//     {
//         return f1.key_.compare( f2.key_ );
//     }
// };
// // Declare bucket type for Michael's hash set
// // The bucket type is any ordered list type like MichaelList, LazyList
// typedef cds::intrusive::MichaelList< cds::gc::HP, Tag,
//     typename cds::intrusive::michael_list::make_traits<
//         // hook option
//         cds::intrusive::opt::hook< cds::intrusive::michael_list::base_hook< cds::opt::gc< cds::gc::HP > > >
//         // item comparator option
//         ,cds::opt::compare< TagCmp >
//     >::type
// >  Foo_bucket;


// // Declare hash functor
// // Note, the hash functor accepts parameter type Tag and std::string
// struct FooHash {
//     size_t operator()( const Tag& f ) const
//     {
//         return cds::opt::v::hash<std::string>()( f.key_ );
//     }
//     size_t operator()( const std::string& f ) const
//     {
//         return cds::opt::v::hash<std::string>()( f );
//     }
// };
// // Michael's set typedef
// typedef cds::intrusive::MichaelHashSet<
//     cds::gc::HP
//     ,Foo_bucket
//     ,typename cds::intrusive::michael_set::make_traits<
//         cds::opt::hash< FooHash >
//     >::type
// > my_set;



//#include <cds/intrusive/lazy_list_nogc.h>
            // Declare item stored in your list
            struct Tag //: public cds::intrusive::lazy_list::node< cds::gc::nogc >
            {
                std::string     key_    ;   // key field
                // ...  other value fields

                Tag(std::string t){
                  this->key_ = t;
                }

                Tag(const Tag& tag)
                {
                  this->key_ = tag.key_;
                }

                friend bool operator==(const Tag& lhs, const Tag& rhs)
                {
                    return lhs.key_.compare(rhs.key_) == 0;
                }
            };

            // Declare comparator for the item
            struct my_compare {
                int operator()( Tag const& i1, Tag const& i2 ) const
                {
                    return i1.key_.compare(i2.key_);
                }
                int operator()( const Tag& d, const std::string& s )
                {
                    return d.key_.compare(s);
                }
                int operator()( const std::string& s, const Tag& d )
                {
                    return s.compare( d.key_ );
                }
            };

            // Declare traits
            // struct my_traits: public cds::intrusive::lazy_list::traits
            // {
            //     typedef cds::intrusive::lazy_list::base_hook< cds::opt::gc< cds::gc::nogc > >   hook;
            //     typedef my_compare compare;
            // };

            // typedef cds::intrusive::LazyList< cds::gc::nogc, Tag, my_traits > my_set;
            typedef std::set<Tag,my_compare> my_set;

            


typedef unsigned char uint8_t;
typedef junction::ConcurrentMap_Grampa<int, my_set*> ConcurrentMap;

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
    turf::Mutex m_rwLock;
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
       long long iterCount = 0;
      for (ConcurrentMap::Iterator iter(set); iter.isValid(); iter.next()) {
        // std::cout << "key: " << iter.getKey() << std::endl;
        iterCount++;
      }
      std::cout << "size: " << iterCount << std::endl;
    }
   
    // 0
    std::string add(std::string e)
    {
      std::string tag = std::to_string(self) + std::to_string(++numAdds);
      return tag;
    }

    void addDownstream(std::string e, std::string tag)
    {
      // try
      // {
      //   my_set* value = set.get(std::stoi(e));
      // }
      // catch (std::invalid_argument iaex)
      // {
        // std::cout << "e: " << e << std::endl;
      // }
      //try {
       // int x = std::stoi(e);
       // x = x + 10;
    //} catch (std::invalid_argument const& ex) {
        //std::cout << "e: " << e << std::endl;
    //}
      

      my_set* value = set.get(std::stoi(e));
      my_set* new_value = new my_set();
      new_value->insert(Tag(tag));
      if(value) {
        for(Tag t : *value)
          new_value->insert(t);
      }
      set.assign(std::stoi(e), new_value);
    }


    std::string remove(std::string e) 
    {
      if(!set.get(std::stoi(e))) return "";
      std::string toRemoveString = "";
      for(Tag t : *set.get(std::stoi(e)))
        toRemoveString += t.key_ + ",";
      return toRemoveString;
    }

    // pre condition: first \subsetequal second
    // output: second - first
    // first: toBeRemoved
    // second: already received and in the set
    std::pair<bool,my_set> setMinusWithGuard(my_set first, my_set* second) {
      if(first.size() > second->size())
        return std::make_pair(false, *second);

      my_set intersection; // second /\ first
      my_set difference; // second - first
      my_set::iterator first1 = first.begin();
      my_set::iterator last1 = first.end();
      my_set::iterator first2 = second->begin();
      my_set::iterator last2 = second->end();
       
      while (first1!=last1 && first2!=last2)
      {
        int compare = my_compare()(*first1, *first2);
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

    void removeDownstream(std::string e, std::string removeSet)
    {
      size_t pos = 0;  
      my_set orderedRemoveSet;
      while (( pos = removeSet.find (',')) != std::string::npos)
      { 
        std::string toRemoveTag = removeSet.substr(0, pos);   
        removeSet.erase(0, pos + 1);
        orderedRemoveSet.insert(Tag(toRemoveTag));
      }

      my_set* value = set.get(std::stoi(e));
      
      if(value){
        // the remove set is bigger than what the node aleady received, returning...
        std::pair<bool,my_set> result = setMinusWithGuard(orderedRemoveSet, value);
        if(result.first)
        {
          my_set* new_value = new my_set();
          for(Tag s : result.second)
            new_value->insert(s);
          
          set.assign(std::stoi(e), new_value);
        }
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

    virtual ReplicatedObject* executeDownstream(MethodCall call, bool b)
    {
      // std::cout << "method: " << static_cast<int>(call.method_type) << "\t" <<  call.arg << std::endl;
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD:
      {
        size_t index = call.arg.find_first_of('-');
        std::string e = call.arg.substr(0, index);
        std::string tag = call.arg.substr(index + 1, call.arg.length());
        addDownstream(e, tag);
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
