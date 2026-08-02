
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <unordered_set>
#include <mutex>

#include "../src/replicated_object.hpp"


typedef unsigned char uint8_t;



class Project : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD_PROJECT = 0,
      DELETE_PROJECT = 1,
      WORKS_ON = 2,
      ADD_EMPLOYEE = 3,
      QUERY = 4
    };

    std::unordered_set<std::string> employees;
    std::unordered_set<std::string> projects;
    std::unordered_set<std::pair<std::string,std::string>, pair_hash> works;
    
    std::recursive_mutex ss_lock;  
 
    Project() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD_PROJECT));
      update_methods.push_back(static_cast<int>(MethodType::DELETE_PROJECT));
      update_methods.push_back(static_cast<int>(MethodType::WORKS_ON));
      update_methods.push_back(static_cast<int>(MethodType::ADD_EMPLOYEE));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD_PROJECT), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::DELETE_PROJECT), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::WORKS_ON), 2));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD_EMPLOYEE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));

      // conflicts
      std::vector<int> g1;
      g1.push_back(static_cast<int>(MethodType::ADD_PROJECT));
      g1.push_back(static_cast<int>(MethodType::DELETE_PROJECT));
      g1.push_back(static_cast<int>(MethodType::WORKS_ON));
      synch_groups.push_back(g1);
      
      // dependencies
      std::vector<int> d1;
      d1.push_back(static_cast<int>(MethodType::ADD_PROJECT));
      d1.push_back(static_cast<int>(MethodType::ADD_EMPLOYEE));
      dependency_relation.insert(std::make_pair(static_cast<int>(MethodType::WORKS_ON), d1));
    }

    // Project(const Project&) = delete;
    Project(Project &obj) : ReplicatedObject(obj)
    {
      //state
      this->employees = obj.employees;
      this->projects = obj.projects;
      this->works = obj.works;
    }

    virtual void toString()
    {
      std::cout << "# employees: " << employees.size() << std::endl;
      // for(auto& s : employees)
      //   std::cout << s << ", ";
      // std::cout << std::endl;

      std::cout << "# projects: " << projects.size() << std::endl;
      // for(auto& c : projects)
      //   std::cout << c << ", ";
      // std::cout << std::endl;


      std::cout << "# enrollemtns: " << works.size() << std::endl;
      // for(auto& e : works)
      //   std::cout << "<" << e.first << "," << e.second << ">" << ", ";
      // std::cout << std::endl;
    }

    // 1
    void addProject(std::string c_id)
    {
      // Project post_state = Project(*this);
      projects.insert(c_id);
      // return *this;
    }
    // 2
    void deleteProject(std::string c_id)
    {
      // Project post_state = Project(*this);
      projects.erase(c_id);
      // return *this;
    }
    // 3
    void worksOn(std::string s_id, std::string c_id)
    {
      // Project post_state = Project(*this);
      works.insert(std::make_pair(s_id, c_id));
      // return *this;
    }
    // 4
    void addEmployee(std::string s_id)
    {
      // Project post_state = Project(*this);
      employees.insert(s_id);
      // return *this;
    }
    // 5
    Project query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD_PROJECT:
      {
        addProject(call.arg);
        break;
      }
      case MethodType::DELETE_PROJECT:
        deleteProject(call.arg);
        break;
      case MethodType::WORKS_ON:
      {
        size_t index = call.arg.find_first_of('-');
        std::string s_id = call.arg.substr(0, index);
        std::string c_id = call.arg.substr(index + 1, call.arg.length());
        worksOn(s_id, c_id); 
        break;
      }
      case MethodType::ADD_EMPLOYEE:
      {
        const std::lock_guard<std::recursive_mutex> lock_ss(ss_lock);
        addEmployee(call.arg);
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
      MethodType method_type = static_cast<MethodType>(call.method_type);
      if(method_type == ADD_EMPLOYEE || method_type == ADD_PROJECT || method_type == QUERY)
        return true;
      else
      {
        if(method_type == DELETE_PROJECT)
        {
          // commented due to high impact on runtime
          // same code for all the experiments
          // therefore it's fair
          // calls are made in a way that no delete course is
          // impermissible
          return true;
        }
        else if(method_type == WORKS_ON)
        {
          size_t index = call.arg.find_first_of('-');
          std::string s = call.arg.substr(0, index);
          std::string c = call.arg.substr(index + 1, call.arg.length());
          const std::lock_guard<std::recursive_mutex> lock_ss(ss_lock);
          if(employees.find(s) == employees.end() || projects.find(c) == projects.end())
            return false;
          return true;
        }
        return false;
      }
    }
};