#include <unordered_map>
#include <cstdlib>
#include <vector>

#include "Thread.cpp"
#include "ServerConnection.cpp"
#include "Socket.cpp"
#include "../wellcoordination/src/replicated_object_crdt.hpp"

using namespace std;
using namespace amirmohsen;

class ServersCommunicationLayer : public Thread
{
	
private:
    int id;
    ReplicatedObject* object;
    std::unordered_map<int,ServerConnection*> connections;
	ServerSocket* acceptingSocket;
    std::vector<string> hosts;
    int* ports;
    std::atomic<int> * initcounter;

    ServerConnection* getConnection(int remoteId);
    void establishConnection(Socket* socket, int remoteId);


public:
	ServersCommunicationLayer(int id, std::vector<string> hosts, int* portNumbers, ReplicatedObject* obj,std::atomic<int> * initcounter);
	
	~ServersCommunicationLayer();

	void broadcast(string &message);
    void broadcast(Buffer* message) ;
	void connectAll();
    void run();
};

ServersCommunicationLayer::~ServersCommunicationLayer() {
    for (auto it : connections)
        delete it.second;
    delete acceptingSocket;
}

ServersCommunicationLayer::ServersCommunicationLayer(int id, std::vector<string> hosts, int* ports, ReplicatedObject* obj,std::atomic<int> * initcounter) 
        {
    this->id = id;
    this->hosts = hosts;
    this->ports = ports;
    this->object = obj;
    this->initcounter=initcounter;
    acceptingSocket = new TCPServerSocket(ports[id - 1]);
    connectAll();
}

void ServersCommunicationLayer::connectAll() {
    for(int i = 1; i <= hosts.size() ; i++) {
        getConnection(i);
    }
}

ServerConnection* ServersCommunicationLayer::getConnection(int remoteId) {
    ServerConnection* ret;
    if(connections.find(remoteId) == connections.end()) {
        ret = new ServerConnection(id, remoteId, NULL, hosts, ports, object, initcounter);
        connections.insert(std::make_pair(remoteId, ret));
    }
    else
        ret = connections.at(remoteId);
    return ret;
}

// broadcasts the message to others (does not send it to self)
void ServersCommunicationLayer::broadcast(string& message) {
	for (auto it : connections) {
		if (getConnection(it.first) != NULL)
			try
			{
                if(it.first != id)
				    getConnection(it.first)->send(message);
			}
			catch (Exception *e)
			{
				cout << e->getMessage() << endl;
				cout << "Unable to send messages to remote " << it.first << endl;
				delete e;
			}
    }
}

void ServersCommunicationLayer::broadcast(Buffer* message) {
	for (auto it : connections) {
		if (getConnection(it.first) != NULL)
			try
			{
                if(it.first != id)
				    getConnection(it.first)->send(message);
			}
			catch (Exception *e)
			{
				cout << e->getMessage() << endl;
				cout << "Unable to send messages to remote " << it.first << endl;
				delete e;
			}
    }
}

void ServersCommunicationLayer::establishConnection(Socket* socket, int remoteId) {
    connections.at(remoteId)->reconnect(socket);
}

void ServersCommunicationLayer::run() {
    while(true) {
        TCPSocket* newSocket = static_cast<TCPSocket*>(acceptingSocket->accept());
        int remoteId = std::stoi(newSocket->getString());
        // std::cout << "Trying establish connection with Replica " << remoteId << std::endl;
        establishConnection(newSocket, remoteId);
    }
}