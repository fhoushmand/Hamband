
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
//openauction 4
//closeauction
//query 6  like movie does not consider read. 
private:
    
public:

    enum MethodType{
      SELL_ITEM = 0,
      STORE_BUY_NOW = 1,
      REGISTER_USER = 2,
      PLACE_BID = 3,
      OPEN_AUCTION=4,
      CLOSE_AUCTION=5,
      QUERY = 6
    };
    std::atomic<int> auction [200][2]={{0}}; //auctionid-user propsed max value-max value //item id eaqual to auciton id
    std::atomic<int> directbuysell [200]= {0};
    std::set<std::int> registeredusers;
    std::set<std::int> openauctions;
    std::set<std::int> closeauctions;
    int userscounter=0;
    //std::set<std::string> movies;
    //std::set<std::string> customers;
    
 
    Rubis() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::SELL_ITEM));
      update_methods.push_back(static_cast<int>(MethodType::STORE_BUY_NOW));
      update_methods.push_back(static_cast<int>(MethodType::REGISTER_USER));
      update_methods.push_back(static_cast<int>(MethodType::PLACE_BID));
      update_methods.push_back(static_cast<int>(MethodType::OPEN_AUCTION));
      update_methods.push_back(static_cast<int>(MethodType::CLOSE_AUCTION));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::SELL_ITEM), 2));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::STORE_BUY_NOW), 2));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REGISTER_USER), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::PLACE_BID), 3));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::OPEN_AUCTION), 2));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::CLOSE_AUCTION), 1));
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

      for (int i = 0; i < 100; i++) {
        directbuysell[i] = 1000; 
        registeredusers.insert(i);
      }
    }

    Rubis(Rubis &obj) : ReplicatedObject(obj)
    {
      //state
      std::memcpy(auction, obj.auction, sizeof(auction));
      std::memcpy(directbuysell, obj.directbuysell, sizeof(directbuysell));
      registeredusers = obj.registeredusers;
      openauctions= obj.openauctions;
      closeauctions= obj.closeAction;
      
      userscounter =obj.userscounter;
    }

    virtual void toString()
    {
      std::cout << "#users_counter: " << userscounter << std::endl;
      //std::cout << "#customer_elements: " << customers.size() << std::endl;
    }

   
    // 0
    void sellItem(int s_id, int value)
    {
      if(registeredusers.count(s_id)==1){
        directbuysell[s_id]=value;
      }
    }
    // 1
    void storeBuyNow(int s_id, int value)
    {
      if (directbuysell[s_id]>value && registeredusers.count(s_id)==1 )
        directbuysell[s_id]=directbuysell[s_id]-value;
    }
    // 2
    void registerUser(int u_id)
    {
      if(registeredusers.count(u_id)==0){
        registeredusers.insert(u_id);
        userscounter++;
      }
    }
    // 3
    void placeBid(int a_id, int u_id, int value)
    {
      if(registeredusers.count(u_id)==1 && openauction.count(a_id)==1 && closeauction.count(a_id)==0){
        if(auction[a_id][1]<value){
          auction[a_id][0]=u_id;
          auction[a_id][1]=value;
        }
      }
    }

    void openAction(int a_id, int stock)
    {
      if(openauction.count(a_id)==0 && closeauction.count(a_id)==0){
        auction[a_id][0]=stock;
        openauction.insert(a_id);
      }
    }
    void closeAction(int a_id)
    {
      if(openauction.count(a_id)==1 && closeauction.count(a_id)==0){
        closeauction.insert(a_id);
      }
    }

    Rubis query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::SELL_ITEM:
      {
        size_t index = call.arg.find_first_of('-');
        int s_id = std::stoi(call.arg.substr(0, index));
        int value = std::stoi(call.arg.substr(index + 1, call.arg.length()));
        sellItem(s_id, value);
        break;
      }
      case MethodType::STORE_BUY_NOW:
      {
        size_t index = call.arg.find_first_of('-');
        int s_id = std::stoi(call.arg.substr(0, index));
        int value = std::stoi(call.arg.substr(index + 1, call.arg.length()));
        storeBuyNow(s_id, value);
        break;
      }
      case MethodType::REGISTER_USER:
      {
        registerUser(std::stoi(call.arg));
        break;
      }
      case MethodType::PLACE_BID:
      {
        size_t index = call.arg.find_first_of('-');
        int a_id = std::stoi(call.arg.substr(0, index));
        std::string sub_arg = call.arg.substr(index + 1, call.arg.length());
        index = sub_arg.find_first_of('-');
        int u_id = std::stoi(sub_arg.substr(0, index));
        int value = std::stoi(sub_arg.substr(index + 1, sub_arg.length()));
        placeBid(a_id, u_id, value);
        break;
      }
      case MethodType::OPEN_AUCTION:
      {
        size_t index = call.arg.find_first_of('-');
        int s_id = std::stoi(call.arg.substr(0, index));
        int stock = std::stoi(call.arg.substr(index + 1, call.arg.length()));
        openAction(s_id, stock);
        break;
      }
      case MethodType::CLOSE_AUCTION:
      {
        closeAction(std::stoi(call.arg));
        break;
      }
      case MethodType::QUERY:
      {
        return this;
        break;
      }
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