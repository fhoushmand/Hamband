
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

class Courseware : public ReplicatedObject
{
private:
    
public:

    enum MethodType{
      ADD_COURSE = 0,
      DELETE_COURSE = 1,
      ENROLL = 2,
      ADD_STUDENT = 3,
      QUERY = 4
    };

    std::unordered_set<std::string> students;
    std::unordered_set<std::string> courses;
    std::unordered_set<std::pair<std::string,std::string>, pair_hash> enrollments;
    
    // std::recursive_mutex ss_lock;  
 
    Courseware() {
      read_methods.push_back(static_cast<int>(MethodType::QUERY));

      update_methods.push_back(static_cast<int>(MethodType::ADD_COURSE));
      update_methods.push_back(static_cast<int>(MethodType::DELETE_COURSE));
      update_methods.push_back(static_cast<int>(MethodType::ENROLL));
      update_methods.push_back(static_cast<int>(MethodType::ADD_STUDENT));

      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD_COURSE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::DELETE_COURSE), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::ENROLL), 2));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::ADD_STUDENT), 1));
      method_args.insert(std::make_pair(static_cast<int>(MethodType::QUERY), 0));

      // conflicts
      std::vector<int> g1;
      g1.push_back(static_cast<int>(MethodType::ADD_COURSE));
      g1.push_back(static_cast<int>(MethodType::DELETE_COURSE));
      g1.push_back(static_cast<int>(MethodType::ENROLL));
      synch_groups.push_back(g1);
      
      // dependencies
      std::vector<int> d1;
      d1.push_back(static_cast<int>(MethodType::ADD_COURSE));
      d1.push_back(static_cast<int>(MethodType::ADD_STUDENT));
      dependency_relation.insert(std::make_pair(static_cast<int>(MethodType::ENROLL), d1));
    }

    // Courseware(const Courseware&) = delete;
    Courseware(Courseware &obj) : ReplicatedObject(obj)
    {
      //state
      this->students = obj.students;
      this->courses = obj.courses;
      this->enrollments = obj.enrollments;
    }

    virtual void toString()
    {
      std::cout << "# students: " << students.size() << std::endl;
      // for(auto& s : students)
      //   std::cout << s << ", ";
      // std::cout << std::endl;

      std::cout << "# courses: " << courses.size() << std::endl;
      // for(auto& c : courses)
      //   std::cout << c << ", ";
      // std::cout << std::endl;


      std::cout << "# enrollemtns: " << enrollments.size() << std::endl;
      // for(auto& e : enrollments)
      //   std::cout << "<" << e.first << "," << e.second << ">" << ", ";
      // std::cout << std::endl;
    }

    // 1
    void addCourse(std::string c_id)
    {
      // Courseware post_state = Courseware(*this);
      courses.insert(c_id);
      // return *this;
    }
    // 2
    void deleteCourse(std::string c_id)
    {
      // Courseware post_state = Courseware(*this);
      courses.erase(c_id);
      // return *this;
    }
    // 3
    void enroll(std::string s_id, std::string c_id)
    {
      // Courseware post_state = Courseware(*this);
      enrollments.insert(std::make_pair(s_id, c_id));
      // return *this;
    }
    // 4
    void registerStudent(std::string s_id)
    {
      // Courseware post_state = Courseware(*this);
      students.insert(s_id);
      // return *this;
    }
    // 5
    Courseware query() { return *this; }


    virtual ReplicatedObject* execute(MethodCall call)
    {
      switch (static_cast<MethodType>(call.method_type))
      {
      case MethodType::ADD_COURSE:
      {
        addCourse(call.arg);
        break;
      }
      case MethodType::DELETE_COURSE:
        deleteCourse(call.arg);
        break;
      case MethodType::ENROLL:
      {
        size_t index = call.arg.find_first_of('-');
        std::string s_id = call.arg.substr(0, index);
        std::string c_id = call.arg.substr(index + 1, call.arg.length());
        enroll(s_id, c_id); 
        break;
      }
      case MethodType::ADD_STUDENT:
      {
        // const std::lock_guard<std::recursive_mutex> lock_ss(ss_lock);
        registerStudent(call.arg);
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
      if(method_type == ADD_STUDENT || method_type == ADD_COURSE || method_type == QUERY)
        return true;
      else
      {
        if(method_type == DELETE_COURSE)
        {
          // commented due to high impact on runtime
          // same code for all the experiments
          // therefore it's fair
          // calls are made in a way that no delete course is
          // impermissible
          return true;
        }
        else if(method_type == ENROLL)
        {
          size_t index = call.arg.find_first_of('-');
          std::string s = call.arg.substr(0, index);
          std::string c = call.arg.substr(index + 1, call.arg.length());
          // const std::lock_guard<std::recursive_mutex> lock_ss(ss_lock);
          if(students.find(s) == students.end() || courses.find(c) == courses.end())
            return false;
          return true;
        }
        return false;
      }
    }

    
};




// int main()
// {
//     std::vector<uint8_t> buffer;
//     buffer.resize(256);
//     uint8_t* b = &buffer[0];

//     CoursewareCall mycall = CoursewareCall("1-1", "enroll 1-10");
//     int* ds = new int[2];
//     ds[0] = 1;
//     ds[1] = 4;
//     int** vs = new int*[2];
//     int vs0[4] = {1,2,3,4};
//     int vs1[4] = {10,20,30,40};
//     vs[0] = vs0;
//     vs[1] = vs1;
//     mycall.setDependencies(ds, vs, 2, 4);
//     mycall.toString();
//     std::cout << "======" << std::endl;
//     auto length = mycall.serialize(b);
//     buffer.resize(length);
//     CoursewareCall backAgain = CoursewareCall::deserialize(b);
//     backAgain.toString();

//     return 0;
// }