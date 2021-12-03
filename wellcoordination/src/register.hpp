
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_set>
#include <mutex>

#include "replicated_object.hpp"


typedef unsigned char uint8_t;

class Register : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      WRITE = 0,
      QUERY = 1
    };

    int reg;
    
    
 
    Register() {
      reg = 0;
      num_methods = 2;
      read_method = 1;
      update_methods.push_back(static_cast<int>(MethodType::WRITE));
    }

    Register(Register &obj) : ReplicatedObject(obj)
    {
      //state
      reg = obj.reg;
    }

    virtual void toString()
    {
      std::cout << "reg: " << reg << std::endl;
    }

   
    // 0
    void write(int a)
    {
      reg = a;
    }
    // 1
    Register query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::WRITE:
        write(std::stoi(call.arg));
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