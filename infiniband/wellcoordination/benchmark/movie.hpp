
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_set>

#include "../src/replicated_object.hpp"


typedef unsigned char uint8_t;

class Movie : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD_MOVIE = 0,
      REMOVE_MOVIE = 1,
      ADD_CUSTOMER = 2,
      REMOVE_CUSTOMER = 3,
      QUERY = 4
    };

    std::set<std::string> movies;
    std::set<std::string> customers;
    
 
    Movie() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD_MOVIE));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE_MOVIE));
      update_methods.push_back(static_cast<int>(MethodType::ADD_CUSTOMER));
      update_methods.push_back(static_cast<int>(MethodType::REMOVE_CUSTOMER));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD_MOVIE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REMOVE_MOVIE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD_CUSTOMER), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::REMOVE_CUSTOMER), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));

      // conflicts
      std::vector<int> g1;
      g1.push_back(static_cast<int>(MethodType::ADD_MOVIE));
      g1.push_back(static_cast<int>(MethodType::REMOVE_MOVIE));
      synch_groups.push_back(g1);
      std::vector<int> g2;
      g2.push_back(static_cast<int>(MethodType::ADD_CUSTOMER));
      g2.push_back(static_cast<int>(MethodType::REMOVE_CUSTOMER));
      synch_groups.push_back(g2);
    }

    Movie(Movie &obj) : ReplicatedObject(obj)
    {
      //state
      movies = obj.movies;
      customers = obj.customers;
    }

    virtual void toString()
    {
      std::cout << "#movie_elements: " << movies.size() << std::endl;
      std::cout << "#customer_elements: " << customers.size() << std::endl;
    }

   
    // 0
    void addMovie(std::string a)
    {
      movies.insert(a);
    }
    // 1
    void removeMovie(std::string a)
    {
      movies.erase(a);
    }
    // 2
    void addCustomer(std::string a)
    {
      customers.insert(a);
    }
    // 3
    void removeCustomer(std::string a)
    {
      customers.erase(a);
    }

    Movie query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD_MOVIE:
        addMovie(call.arg);
        break;
      case MethodType::REMOVE_MOVIE:
        removeMovie(call.arg);
        break;
      case MethodType::ADD_CUSTOMER:
        addCustomer(call.arg);
        break;
      case MethodType::REMOVE_CUSTOMER:
        removeCustomer(call.arg);
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