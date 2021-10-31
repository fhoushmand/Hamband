
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

    BankAccountCall(std::string id, std::string m, std::string arg)
    {
        this->id = id;
        method_name = m;
        if(arg != "")
            args.push_back(arg);
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
        std::cout << "serialized buffer: " << buffer << std::endl;
        
        return os.str().length();
    }


    static BankAccountCall fromStr(std::string const &callStr)
    {
        std::string id;
        std::string name;
        
        size_t index = callStr.find_first_of(' ');
        std::string idAndName = callStr.substr(0, index);

        std::cout << "id and name: " << idAndName << std::endl;
        size_t index_prime = idAndName.find_first_of(':');
        id = idAndName.substr(0, index_prime);
        name = idAndName.substr(index_prime + 1, idAndName.size());
        
        std::string arg;
        std::stringstream ss(callStr.substr(index+1, callStr.size()));
        bool hasArg = false;
        while(ss >> arg){
            hasArg = true;    
        }
        std::cout << "id: " << id << std::endl;
        std::cout << "name: " << name << std::endl;
        if(hasArg)
            std::cout << "arg: " << arg << std::endl;


        

        return BankAccountCall(id, name, hasArg ? arg : "");
    }  
};

int main()
{
    std::vector<uint8_t> buffer;
    buffer.resize(64);
    uint8_t* b = &buffer[0];
    BankAccountCall c = BankAccountCall("1-1", "deposit", "10");
    buffer.resize(c.serialize(b));
    std::cout << "first: " << b << std::endl;
    BankAccountCall::fromStr("1-1:deposit 10").serialize(b);
    std::cout << "second: " << b << std::endl;
    return 0;
}



