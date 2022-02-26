#include <vector>
#include <chrono>

#include "Socket.cpp"
#include "Thread.cpp"
#include "../wellcoordination/src/replicated_object.hpp"

using namespace std;
using namespace amirmohsen;

class ServerConnection
{
private:
	class Receiver : public Thread
	{
		int remoteId;
		Socket *socket;
		ReplicatedObject* object;
	public:
		Receiver(Socket* socket, int remoteId, ReplicatedObject* obj);
		void run();
	};

    class Sender : public Thread
	{
		string *message;
		Socket *socket;
	public:
		Sender(string *message, Socket* socket);
		void run();
	};

	int id;
	int remoteId;
	Socket* socket;
	Receiver* receiver;
	ReplicatedObject* object;
	// Sender* sender;

	std::vector<string> hosts;
	int* ports;

	
    bool isToConnect();
    void closeSocket();
	

public:
	


    ServerConnection(int id, int remoteId, Socket* socket, std::vector<string> hosts, int* ports, ReplicatedObject* obj)
		throw (HostNotFoundException*, GetSocketException*, ConnectException*);
	~ServerConnection();

    void createConnection();
    void send(string& message);
	void send(Buffer* buff);
	void reconnect(Socket* socket);
};

ServerConnection::ServerConnection(int id, int remoteId, Socket* socket, std::vector<string> hosts, int* ports, ReplicatedObject* obj)
		throw (HostNotFoundException*, GetSocketException*, ConnectException*) {
	this->id = id;
	this->remoteId = remoteId;
	this->hosts = hosts;
	this->ports = ports;
	this->socket = socket;
	this->object = obj;
	if(isToConnect())
		createConnection();

	// sender = new Sender();
}

ServerConnection::~ServerConnection() {
	closeSocket();
}

void ServerConnection::send(string& message) {
	socket->send(message);
}

void ServerConnection::send(Buffer* buff) {
	socket->send(to_string(buff->getSize()));
	socket->send(buff);
}

void ServerConnection::createConnection() {
	std::cout << "connecting to " << remoteId << std::endl;
	socket = new TCPSocket(const_cast<char*>(hosts.at(remoteId - 1).c_str()), ports[remoteId - 1]);
	socket->send(to_string(id));

	receiver = new Receiver(socket, remoteId, object);
	receiver->start();
}

bool ServerConnection::isToConnect() {
	return id > remoteId;
}

void ServerConnection::closeSocket() {
	socket->close();
}

// establishing connection only to the nodes with higher id
// connections to the nodes with lower ids have already been created
void ServerConnection::reconnect(Socket* newSocket) {
	std::cout << "reconnecting " << "id: " << id << " remoteId: " << remoteId << std::endl;
	if(socket == NULL) {
		if(isToConnect()){
			createConnection();
		}
		else{
			socket = newSocket;	
			receiver = new Receiver(socket, remoteId, object);
			receiver->start();
		}
	}
	
}

ServerConnection::Receiver::Receiver(Socket* socket, int remoteId, ReplicatedObject* obj) {
	this->socket = socket;
	this->remoteId = remoteId;
	this->object = obj;
}

void ServerConnection::Receiver::run() {
	while(true) {
		if(socket != NULL){
			string lengthStr = socket->getString();
			int length = std::stoi(lengthStr);
			Buffer* buff = socket->receive(length);
			// std::cout << "in receive: " << buff->getContent() << std::endl;
			MethodCall call = object->deserialize(const_cast<char*>(buff->getContent()));
			// std::cout << object->toString(call) << std::endl;
			object->internalExecute(call, remoteId - 1);
		}
		// else {
			// std::this_thread::sleep_for(std::chrono::microseconds(1));
		// }
	}
}



