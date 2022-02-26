#include <cstdlib>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>

#include "ServersCommunicationLayer.cpp"
#include "../wellcoordination/src/counter.hpp"

using namespace std;
using namespace amirmohsen;

int main(int argc, char* argv[])
{
    std::vector<string> hosts;
    hosts.push_back("c01.ib.hpcc.ucr.edu");
    hosts.push_back("c02.ib.hpcc.ucr.edu");
    hosts.push_back("c03.ib.hpcc.ucr.edu");
    hosts.push_back("c04.ib.hpcc.ucr.edu");
    int* ports = new int[4];
    ports[0] = 9000;
    ports[1] = 9001;
    ports[2] = 9002;
    ports[3] = 9003;

    int id = std::stoi(argv[1]);

    bool calculate_throughput = true;
    
    std::string loc =
      "/rhome/fhous001/farzin/FastChain/dory/wellcoordination/workload/";
    loc += std::to_string(4) + "-" + std::to_string(4000000) + "-" +
            std::to_string(static_cast<int>(5));
    loc += "/counter/";

    ReplicatedObject* object = new Counter();
    // if(usecase == "counter") {
        // object = new Counter();
    // }
    // else if(usecase == "register") {
    //     object = new Register();
    // }
    // else if(usecase == "gset") {
    //     object = new GSet();
    // }
    // else if(usecase == "orset") {
    //     object = new ORSet();
    // }
    // else if(usecase == "shop") {
    //     object = new Shop();
    // }
    object->setID(id)->setNumProcess(4)->finalize();
     // start connections
    ServersCommunicationLayer* sc = new ServersCommunicationLayer(id, hosts, ports, object);
    sc->start();
    // wait for all to connect
    std::this_thread::sleep_for(std::chrono::seconds(5));

    int call_id = 0;
    int sent = 0;
    std::string line;
    int expected_calls = 0;
    std::ifstream myfile;
    myfile.open((loc + std::to_string(id) + ".txt").c_str());

    std::vector<MethodCall> requests;
    while (getline(myfile, line)) {
        if (line.at(0) == '#') {
        expected_calls = std::stoi(line.substr(1, line.size()));
        continue;
        }
        std::string sequence_number =
            std::to_string(id) + "-" + std::to_string(call_id++);
        MethodCall call = ReplicatedObject::createCall(sequence_number, line);
        requests.push_back(call);
    }

    std::vector<uint8_t> payload_buffer;
    uint8_t* payload;
    payload_buffer.resize(256);
    payload = &payload_buffer[0];

    std::cout << "started sending..." << std::endl;
    for(MethodCall req : requests) {
        auto length = object->serialize(req, payload);
        std::string message(payload, payload + length);
        payload_buffer.resize(length);
        Buffer* buff = new Buffer();
        buff->setContent(const_cast<char*>(message.c_str()), length);
        if(std::find(object->read_methods.begin(), object->read_methods.end(), req.method_type) == object->read_methods.end()) {
            object->internalExecute(req, id - 1);
            sc->broadcast(buff);
        }
        sent++;
    }
    

    uint64_t local_end = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    std::cout << "issued " << sent << " operations" << std::endl;

    // if(!calculate_throughput){
    //     double sum = 0;
    //     double total_sum = 0;
    //     size_t num = 0;
    //     for (int i = 0; i < object->num_methods; i++) {
    //     total_sum += sum;
    //     sum = 0;
    //     for (auto& pair : response_times[i]){
    //         sum += static_cast<double>(pair.second);
    //         num++;
    //     }
    //     std::cout << "average response time for " << response_times[i].size()
    //                 << " calls to " << i << ": "
    //                 << (sum/1000) / static_cast<int>(response_times[i].size()) << std::endl;
    //     }
    //     std::cout << "total average response time for " << num
    //             << " calls: " << (total_sum/1000) / static_cast<int>(num) << std::endl;
    // }

    // wait for all the ops to arrive and then calculate throughput
    int cs = 0;
    // int sz = static_cast<int>(object->synch_groups.size());
    while (true) {
        cs = 0;
        for (int i = 0; i < object->num_methods; i++)
            for (int x = 0; x < 4; x++) cs += object->calls_applied[i][x];
        if (cs == expected_calls)
            break;
        // std::cout << "received: " << cs << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    uint64_t global_end = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    // std::cout << "throughput:"
    //             << static_cast<double>(num_ops)/static_cast<double>(global_end - local_start) << std::endl;
    object->toString();

    std::this_thread::sleep_for(std::chrono::seconds(60));
    return 0;
}
