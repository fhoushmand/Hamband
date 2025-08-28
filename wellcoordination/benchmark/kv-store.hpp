#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <atomic>

#include "../src/replicated_object.hpp"

typedef unsigned char uint8_t;

class KvStore : public ReplicatedObject
{
public:
    enum MethodType { PUT = 0, GET = 1 };

    std::vector<std::atomic<int>> keysvalues;

    KvStore() : keysvalues(1000000) {
        for (auto &a : keysvalues) a.store(0, std::memory_order_relaxed);

        read_methods.push_back(static_cast<int>(MethodType::GET));
        update_methods.push_back(static_cast<int>(MethodType::PUT));

        method_args.insert(std::make_pair(static_cast<int>(MethodType::PUT), 2));
        method_args.insert(std::make_pair(static_cast<int>(MethodType::GET), 1));

        std::vector<int> g1;
        g1.push_back(static_cast<int>(MethodType::PUT));
        synch_groups.push_back(g1);
    }

    // Copy-ctor: deep-copy atomics by load/store
    KvStore(const KvStore &obj) : ReplicatedObject(obj), keysvalues(obj.keysvalues.size()) {
        for (size_t i = 0; i < keysvalues.size(); ++i) {
            keysvalues[i].store(obj.keysvalues[i].load(std::memory_order_relaxed),
                                std::memory_order_relaxed);
        }
    }

    virtual void toString() {
        std::cout << "keys values size: " << keysvalues.size() << std::endl;
    }

    ~KvStore() {}

    void put(const std::string &key, const std::string &value) {
        int k = std::stoi(key);
        int v = std::stoi(value);
        keysvalues[k].store(v, std::memory_order_relaxed);
    }

    void get(const std::string &key) {
        int k = std::stoi(key);
        int v = keysvalues[k].load(std::memory_order_relaxed);
        //std::cout << "key: " << k << ", value: " << v << std::endl;
        //return *this;
    }

    virtual ReplicatedObject* execute(MethodCall call) {
        switch (static_cast<MethodType>(call.method_type)) {
        case MethodType::PUT: {
            size_t index = call.arg.find_first_of('-');
            std::string key = call.arg.substr(0, index);
            std::string value = call.arg.substr(index + 1);
            put(key, value);
            break;
        }
        case MethodType::GET: {
            std::string key = call.arg;
            get(key);
            break;
        }
            //return this;
        default:
            std::cout << "wrong method name" << std::endl;
            break;
        }
        return this;
    }

    virtual bool isPermissible(MethodCall /*call*/) {
        return true;
    }
};
