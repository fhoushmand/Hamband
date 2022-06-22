#include <cstdlib>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>

#include "ServersCommunicationLayer.cpp"
//#include "ServerConnection.cpp"


#include "../wellcoordination/benchmark/counter-crdt.hpp"
#include "../wellcoordination/benchmark/gset-crdt.hpp"
#include "../wellcoordination/benchmark/orset-crdt.hpp"
#include "../wellcoordination/benchmark/register-crdt.hpp"
#include "../wellcoordination/benchmark/shop-crdt.hpp"

using namespace std;
using namespace amirmohsen;

std::atomic<int> *initcounter;

int main(int argc, char* argv[])
{
    //std::cout << "checkkk111" << std::endl;
    std::vector<string> hosts;
    int id = std::stoi(argv[1]);
    initcounter=new std::atomic<int>;
    initcounter->store(0);
    //std::cout << "checkkk222" << std::endl;
    std::cout <<"id: " <<id<< std::endl;
    int numnodes=std::stoi(argv[2]);
    std::cout <<"nudnum: " <<numnodes<< std::endl;
    int numop=std::stoi(argv[3]);
    std::cout <<"op: " <<numop<< std::endl;
    int writep=std::stoi(argv[4]);
    std::cout <<"writep " <<writep<< std::endl;
    std::string usecase=argv[5];
    std::cout <<"usecase: " <<usecase<< std::endl;

    int* ports = new int[numnodes];
    int baseport=8150+((numnodes+1)*numnodes);
    for(int i=0;i<numnodes;i++)
        ports[i]=baseport+i;


    bool calculate_throughput = true;
    for(int i=6;i<(6+numnodes);i++){
        std::string hoststr=argv[i];
        hoststr=hoststr+".ib.hpcc.ucr.edu";
        //std::cout <<"hotsstr: " <<hoststr<< std::endl;
        hosts.push_back(hoststr);
    }

    std::string loc =
      "/rhome/fhous001/farzin/FastChain/dory/wellcoordination/workload/";
    loc += std::to_string(numnodes) + "-" + std::to_string(numop) + "-" +
            std::to_string(static_cast<int>(writep))+"/";
    loc += usecase;
    loc += "/";
    //std::cout <<"loc: " <<loc<< std::endl;
    ReplicatedObject* object;
    if(usecase == "counter") {
        object = new Counter();
    }
    else if(usecase == "register") {
        object = new Register();
    }
    else if(usecase == "gset") {
        object = new GSet();
    }
    else if(usecase == "orset") {
        object = new ORSet();
    }
    else if(usecase == "shop") {
        object = new Shop();
    }
    object->setID(id)->setNumProcess(numnodes)->finalize();
     // start connections
    ServersCommunicationLayer* sc = new ServersCommunicationLayer(id, hosts, ports, object, initcounter);
    sc->start();
    // wait for all to connect
    std::this_thread::sleep_for(std::chrono::seconds(10));

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
    // begin sync phase-----------------------------------------------------------
    std::cout << "begin sync phase" << std::endl;
    std::string init = "init";
    std::vector<uint8_t> idVector(init.begin(), init.end());
    uint8_t* id_bytes = &idVector[0];
    uint64_t id_len = idVector.size();
    Buffer* initbuff = new Buffer();
    std::string initMsg(id_bytes, id_bytes + id_len);
    //std::cout<<"check2222222"<<std::endl;
    initbuff->setContent(const_cast<char*>(initMsg.c_str()), id_len);
    sc->broadcast(initbuff);
    while(initcounter->load() !=numnodes-1);
    //{
        //std::cout<<initcounter->load()<<std::endl;
        //std::this_thread::sleep_for(std::chrono::seconds(2));
    //}
    std::cout<<"end sync phase"<<initcounter->load()<<std::endl;
    // end sync phase-------------------------------------------------------------
    std::cout << "started sending..." << std::endl;
    uint64_t local_start = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    for(MethodCall req : requests) {
        auto length = object->serializeCRDT(req, payload);
        std::string message(payload, payload + length);
        payload_buffer.resize(length);
        Buffer* buff = new Buffer();
        buff->setContent(const_cast<char*>(message.c_str()), length);
        if(std::find(object->read_methods.begin(), object->read_methods.end(), req.method_type) == object->read_methods.end()) {
            
            req.arg += "-" + object->internalExecuteCRDT(req, id - 1); 
            object->internalDownstreamExecuteCRDT(req, id - 1, false);
            sc->broadcast(buff);
        }
        sent++;
        //std::cout << "issued " << sent << " operations" << std::endl;
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
            for (int x = 0; x < numnodes; x++) cs += object->calls_applied[i][x];
        //std::cout << "received: " << cs << std::endl;
        if (cs == expected_calls)
            break;
        
       std::this_thread::sleep_for(std::chrono::microseconds(10));
       //std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    uint64_t global_end = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    std::cout << "total average response time: "
                << static_cast<double>(local_end - local_start)/(static_cast<double>(numop)/static_cast<double>(numnodes)) << std::endl;
    std::cout << "throughput: "
                << static_cast<double>(numop)/static_cast<double>(global_end - local_start) << std::endl;
    object->toString();

    std::this_thread::sleep_for(std::chrono::seconds(60));
    return 0;
}
