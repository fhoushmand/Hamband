
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>


typedef unsigned char uint8_t;

class BankAccount
{
private:
    
public:
    int balance;
    BankAccount(int b);
    ~BankAccount();

    void withdraw(int a);
    void deposit(int a);
    int query();

    
};

BankAccount::BankAccount(int b) { balance = b;}

BankAccount::~BankAccount() {}


void BankAccount::withdraw(int a) { balance -= a;}
void BankAccount::deposit(int a) { balance += a;}
int BankAccount::query() { return balance;}


struct BankAccountCall
{
    // public
    std::string id;
    std::string call;
    std::string method_name;
		int arg;

		// uint64_t id_len;
    // uint8_t* id_bytes
		// uint64_t call_len;
    // uint8_t* call_bytes;

    uint64_t deps_len;
    bool hasDep = false;
    int* deps;
	
		size_t len;
    
    BankAccountCall() {}

    // BankAccountCall (const BankAccountCall &obj) {
    //   std::cout << "copy constructor called" << std::endl;
    //   len = obj.len;
      
    //   id_len = obj.id_len;
    //   id = new uint8_t[id_len];
    //   memcpy(id, obj.id, id_len);

    //   call_len = obj.call_len;
    //   call = new uint8_t[call_len];
    //   memcpy(call, obj.call, call_len);

    //   extractCall();
    // }
		

    BankAccountCall(std::string id, std::string call)
    {
        this->id = id;
				// this->id_len = id_len;
				this->call = call;
				// this->call_len = call_len;
				// this->len = 0;
				extractCall();
    }

    void setDeps(int* deps, uint64_t deps_len) {
      hasDep = true;
      this->deps = deps;
      this->deps_len = deps_len;
    }

    size_t serialize(uint8_t* buffer) {
      
      
      std::vector<uint8_t> idVector(id.begin(), id.end());
      uint8_t *id_bytes = &idVector[0];
      uint64_t id_len = idVector.size();

      std::vector<uint8_t> callVector(call.begin(), call.end());
      uint8_t *call_bytes = &callVector[0];
      uint64_t call_len = callVector.size();
      
      // std::cout << "call bytes: " << call_bytes << std::endl;
      // std::cout << "call bytes size: " << call_len << std::endl;

			uint8_t* start = buffer + sizeof(uint64_t);
			auto temp = start;



      *reinterpret_cast<uint64_t*>(start) = id_len;
      start += sizeof(id_len);

      *reinterpret_cast<uint64_t*>(start) = call_len;
      start += sizeof(call_len);

			*reinterpret_cast<uint64_t*>(start) = deps_len;
      start += sizeof(deps_len);

      memcpy(start, id_bytes, id_len);
      start += id_len;

			memcpy(start, call_bytes, call_len);
      start += call_len;

      if(hasDep){
        *reinterpret_cast<int*>(start) = 1;
        start += sizeof(1);
        for(size_t i = 0; i < deps_len; i++)
        {
          *reinterpret_cast<int*>(start) = deps[i];
          start += sizeof(deps[i]);
        }
      }
      else
      {
        *reinterpret_cast<int*>(start) = 0;
        start += sizeof(0);
      }

			len += start - temp;

      auto length = reinterpret_cast<uint64_t*>(start - len - sizeof(uint64_t));
      *length = len;
      return len + sizeof(uint64_t) + 3*sizeof(uint64_t)  + (deps_len + 1)*sizeof(int);
    }

    inline void extractCall()
    {
      size_t spaceIndex = call.find_first_of(' ');
      
      if(spaceIndex == std::string::npos)
        method_name = call;
      else {
				method_name = call.substr(0, spaceIndex);
        arg = std::stoi(call.substr(spaceIndex + 1, call.size()));
      }
      // std::cout << callStr.substr(spaceIndex + 1, callStr.size()) << std::endl;;
    }


    static BankAccountCall deserialize(uint8_t* buffer)
    {
			uint64_t len = *reinterpret_cast<uint64_t*>(buffer);
			// std::cout << "-tot_size: " << len << std::endl;

			uint64_t id_len = *reinterpret_cast<uint64_t*>(buffer + sizeof(uint64_t));
			// std::cout << "-id_size: " << id_len << std::endl;

			uint64_t call_len = *reinterpret_cast<uint64_t*>(buffer + 2*sizeof(uint64_t));
			// std::cout << "-call_size: " << call_len << std::endl;

			uint64_t deps_len = *reinterpret_cast<uint64_t*>(buffer + 3*sizeof(uint64_t));
			// std::cout << "-deps_size: " << deps_len << std::endl;

			size_t id_offset = 4*sizeof(uint64_t);
      uint8_t* id_bytes = new uint8_t[id_len];
      memcpy(id_bytes, buffer + id_offset, id_len);
      std::string id(id_bytes, id_bytes + id_len);

      // std::cout << "-id: " << id << std::endl;

			size_t call_offset = id_offset + id_len;
			uint8_t* call_bytes = new uint8_t[call_len];
      memcpy(call_bytes, buffer + call_offset, call_len);
      std::string call(call_bytes, call_bytes + call_len);

      // std::cout << "-call: " << call << std::endl;
			
			int methodd = *reinterpret_cast<int*>(buffer + call_offset + call_len);
			// std::cout << "-method: " << methodd << std::endl;
      BankAccountCall out = BankAccountCall(id, call);
      int* deps = new int[deps_len];
			for(size_t i = 0; i < deps_len; i++)
			{
				deps[i] = *reinterpret_cast<int*>(buffer + call_offset + call_len + (i+1)*sizeof(int));
        // std::cout << deps[i] << ", ";
			}
      out.setDeps(deps, deps_len);
			return out;
    }

    bool isPermissible(BankAccount preState)
    {
      if(method_name == "deposit" || method_name == "query")
        return true;
      else
        return preState.balance >= arg;
    }  

    void toString()
    {
      std::cout << "method: " << method_name << std::endl;
      std::cout << "arg: " << arg << std::endl;
      std::cout << id << std::endl;
      std::cout << call << std::endl;
      // int methodd = *reinterpret_cast<int*>(b + 3*sizeof(uint64_t) + call_size + id_size);
      // std::cout << "method: " << methodd << std::endl;

      for(size_t i = 0; i < deps_len; i++)
      {
        std::cout << deps[i] << ", ";
      }
      std::cout << std::endl;
    }
};


// int main()
// {
//     std::vector<uint8_t> buffer;
//     buffer.resize(128);
//     uint8_t* b = &buffer[0];

//     BankAccountCall mycall = BankAccountCall("1-1", "deposit 10");
//     mycall.toString();
//     std::cout << "======" << std::endl;
//     auto length = mycall.serialize(b);
//     buffer.resize(length);
//     BankAccountCall backAgain = BankAccountCall::deserialize(b);
//     backAgain.toString();

//     return 0;
// }














/*
** methods are mapped to a zero-based integer
** query: 0
** deposit: 1
** withdraw: 2
*/
// class Entry {
//    public:
//     Entry() {}

//     Entry(uint8_t* start)
//         : base{start},
//           start{start + sizeof(uint64_t)},
//           len{0} {}

//     inline void fast_store(uint64_t id_len, uint8_t* id_buf, uint64_t call_len,
//                            uint8_t* call_buf, int method, int* dep, size_t size) {
//       auto temp = start;

//       *reinterpret_cast<uint64_t*>(start) = id_len;
//       start += sizeof(id_len);

//       *reinterpret_cast<uint64_t*>(start) = call_len;
//       start += sizeof(call_len);

//       *reinterpret_cast<uint64_t*>(start) = 3;
//       start += sizeof(uint64_t);

//       memcpy(start, id_buf, id_len);
//       start += id_len;

// 			memcpy(start, call_buf, call_len);
//       start += call_len;

//       // per method dependency
// 			*reinterpret_cast<int*>(start) = method;
//       start += sizeof(method);
// 			for(size_t i = 0; i < size; i++)
// 			{
// 				*reinterpret_cast<int*>(start) = dep[i];
//       	start += sizeof(dep[i]);
// 			}


// 			len += start - temp;

//     }

//     inline size_t finalize() {
//       auto length = reinterpret_cast<uint64_t*>(start - len - sizeof(uint64_t));
//       *length = len;
//       *start = 0xff;

//       // The +1 is for the canary value, the uint64_t is because we encode the
//       // the length.
//       return len + 1 + sizeof(uint64_t) + 4*sizeof(int) + 1;
//     }

//     inline uint8_t* basePtr() const { return base; }

//     inline size_t length() const { return len + 1 + sizeof(uint64_t); }

//    private:
//     uint8_t* base;
//     uint8_t* start;
//     uint64_t len;
//   };
