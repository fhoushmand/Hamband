
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

class Register : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      WRITE = 0,
      QUERY = 1
    };

    int local_value;
    int local_timestamp;
    
    int value;
    int timestamp;
    
   
    Register() {
      value = 0;

      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::WRITE));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::WRITE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));
    }

    Register(Register &obj) : ReplicatedObject(obj)
    {
      //state
      this->value = obj.value;
      this->timestamp = obj.timestamp;

      this->local_value = obj.value;
      this->local_timestamp = obj.timestamp;
    }
   
    // 0
    std::string write(int val)
    {
      local_timestamp += 1;
      local_value = val;
      return std::to_string(local_timestamp);
    }

    //downStream
    void writeDownstream(int val, int ts)
    {
      if (timestamp < ts){
        timestamp = ts;
        value = val;
      }
    }
    // 1
    Register query() { 
      if(local_timestamp > timestamp)
      {
        value = local_value;
        timestamp = local_timestamp;
      }
      return *this;
    }


    virtual std::string execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::WRITE:
      {
        size_t index = call.arg.find_first_of('-');
        std::string val = call.arg.substr(0, index);
        return write(std::stoi(val));
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
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::WRITE:
      {
        size_t index = call.arg.find_first_of('-');
        std::string val = call.arg.substr(0, index);
        std::string ts = call.arg.substr(index + 1, call.arg.length());
        writeDownstream(std::stoi(val), std::stoi(ts));
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

    virtual void toString()
    {
      std::cout << "reg: " << value << std::endl;
    }



    virtual bool isPermissible(MethodCall call)
    {
        return true;
    }
};