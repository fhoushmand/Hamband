
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

class PNSet : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD = 0,
      REMOVE = 1,
      QUERY = 2
    };
    std::recursive_mutex m;
    int pnsetsource[200001][2]={{0}};
    int pnsetremote[200001][2]={{0}};
    int arraysizesource=0;
    int arraysizeremote=0;
    int setsizesource=0;
    int setsizeremote=0;
    /// -------------------------------------------------------------
    
    
 
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
      pnsetsource= obj.pnsetsource;
      pnsetremote= obj.pnsetremote;
      arraysizesource= obj.arraysizesource;
      arraysizeremote= obj.arraysizeremote;
      setsizesource=obj.setsizesource;
      setsizeremote=obj.setsizeremote;
      ///------------------------------------------------------------------------

    }

    virtual void toString()
    {
      std::cout << "#elements: " << (setsizesource+setsizeremote) << std::endl;
      //--------------------------------------------------------------------------
  
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
      bool find= false;
      //std::scoped_lock lock(m);
      if (b==false){
        find=false;
        for(int i=0; i<arraysizesource; i++){
          if(pnsetsource[i][0]==std::atoi(a)){
            pnsetsource[i][1]++;
            setsizesource++;
            find=true;
            break;
          }
        }
        if(!find){
          pnsetsource[arraysizesource][0]= std::atoi(a);
          pnsetsource[arraysizesource][1]++;
          arraysizesource++;
          setsizesource++;
        }
      //---------------------------------------------------
      }
      else{
        find=false;
        for(int i=0; i<arraysizeremote; i++){
          if(pnsetremote[i][0]==std::atoi(a)){
            pnsetremote[i][1]++;
            setsizeremote++;
            find=true;
            break;
          }
        }
        if(!find){
          pnsetremote[arraysizeremote][0]= std::atoi(a);
          pnsetremote[arraysizeremote][1]++;
          arraysizeremote++;
          setsizeremote++;
        }

      //---------------------------------------------------
      }
      return "";
    }
    std::string removeDownstream(std::string a, bool b)
    {
      //std::scoped_lock lock(m);
      bool find= false;
      //std::scoped_lock lock(m);
      if (b==false){
        find=false;
        for(int i=0; i<arraysizesource; i++){
          if(pnsetsource[i][0]==std::atoi(a)){
            pnsetsource[i][1]--;
            setsizesource--;
            find=true;
            break;
          }
        }
        if(!find){
          pnsetsource[arraysizesource][0]= std::atoi(a);
          pnsetsource[arraysizesource][1]--;
          arraysizesource++;
          setsizesource--;
        }
      //---------------------------------------------------
      }
      else{
        find=false;
        for(int i=0; i<arraysizeremote; i++){
          if(pnsetremote[i][0]==std::atoi(a)){
            pnsetremote[i][1]--;
            setsizeremote--;
            find=true;
            break;
          }
        }
        if(!find){
          pnsetremote[arraysizeremote][0]= std::atoi(a);
          pnsetremote[arraysizeremote][1]--;
          arraysizeremote++;
          setsizeremote--;
        }

      //---------------------------------------------------
      }
      return "";
    }
    // 1
    PNSet query() { return *this; }


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