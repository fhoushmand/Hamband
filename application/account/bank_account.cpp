
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>



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
    std::string method_name;
    std::vector<std::string> args;

    BankAccountCall(std::string m, std::string arg)
    {
        method_name = m;
        if(arg != "")
        args.push_back(arg);
    }

    std::string serialize() {
        std::ostringstream os;

        os << std::hex << method_name << " ";
        for(std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++ )
        {
            os << *it;
        }
        return os.str();
    }

    static BankAccountCall fromStr(std::string const &callStr)
    {
        std::string res(callStr);
        std::stringstream ss(res);

        std::string name;

        ss >> std::hex >> name;

        std::string arg;
        bool hasArg = false;
        while(ss >> arg){
            hasArg = true;    
        }

        return BankAccountCall(name, hasArg ? arg : "");
    }  
};

int main()
{
    BankAccountCall c = BankAccountCall("withdraw", "10");
    std::cout << c.serialize() << std::endl;
    std::cout << BankAccountCall::fromStr(c.serialize()).serialize() << std::endl;
    return 0;
}



