
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
    /* data */
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
    std::string id;
    std::string method_name;
    std::vector<std::string> args;
    int* deps;

    BankAccountCall(std::string id, std::string m, std::string arg)
    {
        this->id = id;
        method_name = m;
        if(arg != "")
            args.push_back(arg);
    }

    void setDeps(int* ds)
    {
        deps = ds;
    }


    size_t serialize(uint8_t* buffer) {
        std::ostringstream os;
        
        os << id << ":" << method_name << " ";
        for(std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++ )
        {
            os << *it;
        }
        os << "\0";
        std::strcpy(reinterpret_cast<char*>(buffer), os.str().c_str());
        return os.str().length();
    }


    static BankAccountCall fromStr(std::string const &callStr)
    {
        std::string id;
        std::string name;
        
        size_t index = callStr.find_first_of(' ');
        std::string idAndName = callStr.substr(0, index);

        size_t index_prime = idAndName.find_first_of(':');
        id = idAndName.substr(0, index_prime);
        name = idAndName.substr(index_prime + 1, idAndName.size());
        
        std::string arg;
        std::stringstream ss(callStr.substr(index+1, callStr.size()));
        bool hasArg = false;
        while(ss >> arg){
            hasArg = true;    
        }
        return BankAccountCall(id, name, hasArg ? arg : "");
    }

    bool isPermissible(BankAccount preState)
    {
        if(method_name == "deposit" || method_name == "query")
			return true;
		else
			return preState.balance >= std::stoi(args[0]);
    }  
};


// class ParsedCallAccount {
//  public:
//   ParsedCall() {}
//   ParsedCall(uint8_t* ptr) : ptr{ptr} {}

//   inline int* dependencies() {
//     return reinterpret_cast<int*>(ptr + offsets[2]);
//   }

//   inline bool hasDeps() {
//     return *reinterpret_cast<int*>(ptr + offsets[1]);
//   }

//   inline void setDependencies(int* deps) {

//     int* first = deps;
//     int* last = deps + 4;
//     int* d_first = reinterpret_cast<int*>(ptr + offsets[1]);

//     while (first != last) {
//         *d_first++ = *first++;
//     }
//   }

//   inline std::pair<uint8_t*, size_t> payload() {
//     if(hasDeps()){
//       auto length = *reinterpret_cast<uint64_t*>(ptr + offsets[0]) - 4 * sizeof(int);
//       auto buf = ptr + offsets[2] + (4 * sizeof(int));  
//       return std::make_pair(buf, length);
//     }
//     else{
//       auto length = *reinterpret_cast<uint64_t*>(ptr + offsets[0]) - sizeof(int);
//       auto buf = ptr + offsets[2];
//       return std::make_pair(buf, length);
//     }
//   }

//   inline bool isPopulated() { return *reinterpret_cast<uint64_t*>(ptr) > 0; }

//   // inline size_t totalLength() {
//   //   return *reinterpret_cast<uint64_t*>(ptr) + sizeof(uint64_t) + 1;
//   // }

//   // static inline size_t copy(uint8_t* dst, uint8_t* src) {
//   //   auto src_len = *reinterpret_cast<uint64_t*>(src);
//   //   auto size = src_len + 1 + sizeof(uint64_t);
//   //   if (src != dst) {
//   //     memcpy(dst, src, size);
//   //   }

//   //   return size;
//   // }

//  private:
//   static constexpr const int offsets[] = {
//       0,   // For the length of the entry,
//       8,   // Flag for deps,
//       12  // Deps/buffer,
//   };

//   uint8_t* ptr;
}

// int main()
// {
//     std::vector<uint8_t> buffer;
//     buffer.resize(64);
//     uint8_t* b = &buffer[0];
//     BankAccountCall c = BankAccountCall("1-1", "deposit", "10");
//     buffer.resize(c.serialize(b));
//     std::cout << "first: " << b << std::endl;
//     BankAccountCall::fromStr("1-1:deposit 10").serialize(b);
//     std::cout << "second: " << b << std::endl;
//     return 0;
// }



