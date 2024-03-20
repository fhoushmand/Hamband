
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_set>

#include "../src/replicated_object.hpp"


typedef unsigned char uint8_t;

class Rubis : public ReplicatedObject
{

//sellItem    0   2(id+value)
//storeBuyNow 1   2(id+value) by leader 
//registerUser 2   1(id)       by leader
//openAuction    1(id)        by default consider 100 auctions are open. 
//placeBid 3      3(auctionid+userid+value)
//query 4  like movie does not consider read. 
private:
    
public:

    enum MethodType{
      SELL_ITEM = 0,
      STORE_BUY_NOW = 1,
      REGISTER_USER = 2,
      PLACE_BID = 3,
      QUERY = 4
    };
    std::atomic<int> auction [100][3]={{0}}; //auctionid-quantity-user propsed max value-max value 
    bool registeredusers [100]={false};
    int userscounter=0;
    //std::set<std::string> movies;
    //std::set<std::string> customers;
    
 
    Rubis() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::SELL_ITEM));
      update_methods.push_back(static_cast<int>(MethodType::STORE_BUY_NOW));
      update_methods.push_back(static_cast<int>(MethodType::REGISTER_USER));
      update_methods.push_back(static_cast<int>(MethodType::PLACE_BID));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::SELL_ITEM), 2));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::STORE_BUY_NOW), 2));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REGISTER_USER), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::PLACE_BID), 3));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));

      // conflicts
      std::vector<int> g1;
      g1.push_back(static_cast<int>(MethodType::STORE_BUY_NOW));
      //g1.push_back(static_cast<int>(MethodType::REMOVE_MOVIE));
      synch_groups.push_back(g1);
      std::vector<int> g2;
      g2.push_back(static_cast<int>(MethodType::REGISTER_USER));
      //g2.push_back(static_cast<int>(MethodType::REMOVE_CUSTOMER));
      synch_groups.push_back(g2);
    }

    Rubis(Rubis &obj) : ReplicatedObject(obj)
    {
      //state
      std::memcpy(auction, obj.auction, sizeof(auction));
      std::memcpy(registeredusers, obj.registeredusers, sizeof(registeredusers));
      userscounter =obj.userscounter;
      //auction = obj.auction;
      //registeredusers = obj.registeredusers;
    }

    virtual void toString()
    {
      std::cout << "#users_counter: " << userscounter << std::endl;
      //std::cout << "#customer_elements: " << customers.size() << std::endl;
    }

   
    // 0
    void sellItem(int s_id, int value)
    {
      auction[s_id][0]=value;
    }
    // 1
    void storeBuyNow(int s_id, int value)
    {
      if (auction[s_id][0]>value)
        auction[s_id][0]=auction[s_id][0]-value;
    }
    // 2
    void registeruser(int u_id)
    {
      registeredusers[u_id]=true;
      userscounter++;
    }
    // 3
    void placeBid(int a_id, int u_id, int value)
    {
      if(registeredusers[u_id]){
        if(auction[a_id][2]<value){
          auction[a_id][1]=u_id;
          auction[a_id][2]=value;
        }
      }
    }

    Rubis query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::SELL_ITEM:
        size_t index = call.arg.find_first_of('-');
        int s_id = std::stoi(call.arg.substr(0, index));
        int value = std::stoi(call.arg.substr(index + 1, call.arg.length()));
        sellItem(s_id, value);
        break;
      case MethodType::STORE_BUY_NOW:
        size_t index = call.arg.find_first_of('-');
        int s_id = std::stoi(call.arg.substr(0, index));
        int value = std::stoi(call.arg.substr(index + 1, call.arg.length()));
        storeBuyNow(s_id, value);
        break;
      case MethodType::REGISTER_USER:
        registeruser(std::stoi(call.arg));
        break;
      case MethodType::PLACE_BID:
        size_t index = call.arg.find_first_of('-');
        int a_id = std::stoi(call.arg.substr(0, index));
        std::string sub_arg = call.arg.substr(index + 1, call.arg.length());
        index = sub_arg.find_first_of('-');
        int u_id = std::stoi(sub_arg.substr(0, index));
        int value = std::stoi(sub_arg.substr(index + 1, sub_arg.length()));
        placeBid(a_id, u_id, value)
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