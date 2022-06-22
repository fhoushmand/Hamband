/*
Developed by:
	Mohsen Lesani
	Amir Moghimi
This program is under GNU GPL (for more information see http://www.gnu.org).
*/

#ifndef AMIR_MOHSEN_SOCKET_CPP
#define AMIR_MOHSEN_SOCKET_CPP

#include "Exception.cpp"

#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <errno.h>

using namespace std;

namespace amirmohsen {

class EndOfBufferException : public Exception
{
public:
	EndOfBufferException(std::string m) : Exception(m) {};
};

class HostNotFoundException : public Exception
{
public:
	HostNotFoundException(std::string m) : Exception(m) {};
};

class GetSocketException : public Exception
{
public:
	GetSocketException(std::string m) : Exception(m) {};
};

class SocketClosedException : public Exception
{
public:
	SocketClosedException(std::string m) : Exception(m) {};
};

class ReceiveException : public Exception
{
public:
	ReceiveException(std::string m) : Exception(m) {};
};

class BindException : public Exception
{
public:
	BindException(std::string m) : Exception(m) {};
};

class AcceptException : public Exception
{
public:
	AcceptException(std::string m) : Exception(m) {};
};

class ConnectException : public Exception
{
public:
	ConnectException(std::string m) : Exception(m) {};
};

//-------------------------------------------------------------------------------

/*
* Do delete every pointer you get from any method and exceptions you catch.
* An example is avaiable in .cpp file.
*/

class Buffer
{
private:
	static const int DEFAULT_SIZE;

	char *content;
	int size;
	int addCursor;
	int getCursor;

	void fitSize(int size)
	{
		if (size > this->size)
		{
			char* newContent = new char[size];
			for (int i = 0; i < this->size; i++)
				newContent[i] = content[i];
			this->size = size;
			delete [] content;
			content = newContent;
		}
	}

public:
	Buffer(int size = DEFAULT_SIZE)
	{
		this->size = size;
		content = new char[size];
		addCursor = 0;
		getCursor = 0;
	}

	~Buffer()
	{
		delete [] content;
	}

	void resetCursor()
	{
		getCursor = 0;
	}

	int getSize()
	{
		return addCursor;
	}

	void setContent(char *newContent, int size)
	{
		fitSize(size);
		for (int i = 0; i < size; i++)
			content[i] = newContent[i];
		addCursor = size;
		getCursor = 0;
	}

	char *getContent()
	{
		char *returnContent = new char[addCursor];
		for (int i = 0; i < addCursor; i++)
			returnContent[i] = content[i];
		return returnContent;
	}

	void add(char val)
	{
		if (size - addCursor < sizeof(char))
			fitSize(size + size/2 + sizeof(char));
		for(int i = 0; i < sizeof(val); i++)
			content[addCursor++] = (char)(((0x00FF << (8 * i)) & val) >> (8 * i));
	}

	void add(int val)
	{
		if (size - addCursor < sizeof(int))
			fitSize(size + size/2 + sizeof(int));
		for(int i = 0; i < sizeof(val); i++)
			content[addCursor++] = (char)(((0x00FF << (8 * i)) & val) >> (8 * i));

	}

	void add(long val)
	{
		if (size - addCursor < sizeof(long))
			fitSize(size + size/2 + sizeof(long));
		for(int i = 0; i < sizeof(val); i++)
			content[addCursor++] = (char)(((0x00FF << (8 * i)) & val) >> (8 * i));
	}

	void add(const char* str)
	{
		if (size - addCursor < strlen(str) + 1)
			fitSize(size + size/2 + strlen(str) + 1);
		int i = 0;
		while (str[i])
			content[addCursor++] = str[i++];
		content[addCursor++] = 0;
	}

	void add(string str)
	{
		add(str.c_str());
	}

	char getChar() //throw (EndOfBufferException*)
	{
		if (addCursor - getCursor < sizeof(char))
			throw new EndOfBufferException(" @ Buffer.getChar(): End of Buffer!");
		char val = 0;
		for(int i = 0; i < sizeof(char); i++)
			val += (((content[getCursor++] << (8 * i))) & (0x0FF << 8 * i));
		return val;
	}

	int getInt() //throw (EndOfBufferException*)
	{
		if (addCursor - getCursor < sizeof(int))
			throw new EndOfBufferException(" @ Buffer.getInt(): End of Buffer!");
		int val = 0;
		for(int i = 0; i < sizeof(int); i++)
			val += (((content[getCursor++] << (8 * i))) & (0x0FF << 8 * i));
		return val;
	}

	long getLong() //throw (EndOfBufferException*)
	{
		if (addCursor - getCursor < sizeof(long))
			throw new EndOfBufferException(" @ Buffer.getLong(): End of Buffer!");
		long val = 0;
		for(int i = 0; i < sizeof(long); i++)
			val += (((content[getCursor++] << (8 * i))) & (0x0FF << 8 * i));
		return val;
	}

	string getString() //throw (EndOfBufferException*)
	{
		string val;
		while (content[getCursor])
		{
			if (getCursor >= addCursor)
				throw new EndOfBufferException(" @ Buffer.getString(): End of Buffer!");
			val += content[getCursor++];
		}
		getCursor++;
		return val;
	}
};

const int Buffer::DEFAULT_SIZE = 6;

//-------------------------------------------------------------------------------

class ServerSocket;
class TCPServerSocket;// : public ServerSocket;
class UDPServerSocket;// : public ServerSocket;

class Socket
{
public:
	virtual void send(Buffer *buffer) = 0;
	virtual void send(string message) = 0;
	virtual Buffer *receive(int size) = 0;
	virtual string getString() = 0;
	virtual void close() = 0;
	virtual ~Socket() {};
};

class TCPSocket : public Socket
{

friend class TCPServerSocket;

private:
	int sd;
	TCPSocket(int sd) { this->sd = sd; }

public:
	TCPSocket(char *host, int port)
		//throw (HostNotFoundException*, GetSocketException*, ConnectException*)
	{
		struct hostent *hp;
		hp = gethostbyname(host);
		if (!hp) throw new HostNotFoundException(" @ TCPSocket.TCPSocket(): Error in gethostbyname()!");

		struct sockaddr_in sin;
		bzero((char *)&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
		sin.sin_port = htons(port);

		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
			throw new GetSocketException(" @ TCPSocket.TCPSocket(): Error in socket()!");

		if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		{
			::close(sd);
			std::cout << "print: " << strerror(errno) << std::endl;
			throw new ConnectException(" @ TCPSocket.TCPSocket(): Error in connect()!");
		}
	}

	~TCPSocket()
	{
		::close(sd);
	}

	void close()
	{
		::close(sd);
	}

	void send(Buffer *buffer)
	{
		char *content = buffer->getContent();
		int sentCount = 0;
		int size = buffer->getSize();
		while (sentCount < size)
			sentCount += ::send(sd, (void *)(content + sentCount), (size_t)(size - sentCount), 0);
		delete [] content;
	}

	void send(string message)
	{
		// std::cout << "send" << std::endl;
		int sentCount = 0;
		int size = message.length() + 1;
		// std::cout << "size: " << size << std::endl;
		while (sentCount < size)
			sentCount += ::send(sd, (void *)(message.c_str() + sentCount), size - sentCount, 0);
	}

	Buffer *receive(int size) //throw (SocketClosedException*, ReceiveException*)
	{
		char *content = new char[size];

		int receivedCount = 0;

		while (receivedCount < size)
		{
			 int val = recv(sd, content + receivedCount, size - receivedCount, 0);
			 if (val == 0) throw new SocketClosedException(" @ TCPSocket.receive(): The other side closed the socket!");
			 if (val < 0) throw new ReceiveException(" @ TCPSocket.receive(): Error in recv()!");
			 receivedCount += val;
		}

		Buffer *buffer = new Buffer(size);
		buffer->setContent(content, size);
		delete [] content;

		return buffer;
	}

	string getString() //throw (SocketClosedException*, ReceiveException*)
	{
		string message;
		char c = 1;
		while (c != 0)
		{
			 int val = recv(sd, &c, 1, 0);
			 if (val == 0) throw new SocketClosedException(" @ TCPSocket.getString(): The other side closed the socket!");
			 if (val < 0) throw new ReceiveException(" @ TCPSocket.getString(): Error in recv()!");
			 if (c) message += c;
		}
		return message;
	}
};

/*class UDPSocket : public Socket
{
};*/

class ServerSocket
{
public:
	virtual Socket *accept() = 0;
	virtual ~ServerSocket() {};
};

class TCPServerSocket : public ServerSocket
{
private:
	int sd;
public:
	TCPServerSocket(int port, int maxPending = 10) //throw (GetSocketException*, BindException*)
	{
		struct sockaddr_in sin;
		bzero((char *)&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(port);

		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
			throw new GetSocketException(" @ TCPServerSocket.TCPServerSocket(): Error in socket()!");

		if (bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		{
			close(sd);
			throw new BindException(" @ TCPServerSocket.TCPServerSocket(): Error in bind()!");
		}
		listen(sd, maxPending);
	}

	~TCPServerSocket()
	{
		close(sd);
	}

	Socket *accept() //throw (AcceptException*)
	{
		struct sockaddr_in sin;
		socklen_t len = sizeof(sin);

		int newSd = ::accept(sd, (struct sockaddr *)&sin, &len);
		if (newSd < 0)
			throw new AcceptException(" @ TCPServerSocket.accept(): Error in accept()!");
		return new TCPSocket(newSd);
	}

};

/*class UDPServerSocket : public ServerSocket
{
};*/

} // namespace amirmohsen


//-------------------------------------------------------------------------------

//Socket test
/*
using namespace std;
using namespace amirmohsen;

int main(int argc, char* argv[])
{
	const int PORT = 2777;
	Socket *senderSocket = NULL;
	ServerSocket *acceptingSocket = NULL;
	try
	{
		// Server Socket
		acceptingSocket = new TCPServerSocket(PORT);
		// Client Socket
		senderSocket = new TCPSocket("localhost", PORT);
	}
	catch (Exception *e)
	{
		cout << e->getMessage() << endl;
		delete e;
		delete acceptingSocket;
		delete senderSocket;
		exit(1);
	}

	Buffer *senderBuffer = new Buffer();

	senderBuffer->add("A Test String");
	char *bufferContent = senderBuffer->getContent();
	cout << "First test send buffer content: " << bufferContent << endl;
	cout << "Buffer size: " << senderBuffer->getSize() << endl << endl;
	delete [] bufferContent;

	cout << "Adding an int(128), a char('B') and a long(777)." << endl;

	senderBuffer->add(128);
	senderBuffer->add('B');
	senderBuffer->add((long) 777);

	bufferContent = senderBuffer->getContent();
	cout << "Buffer Size: " << senderBuffer->getSize() << endl << endl;
	delete [] bufferContent;

	senderSocket->send(senderBuffer);
	cout << "Buffer sent." << endl << endl;

	cout << "Second test send buffer content: two chars '\\0\\1'" << endl;

	senderBuffer->setContent("\0\1", 2);
	bufferContent = senderBuffer->getContent();
	cout << "Buffer Size: " << senderBuffer->getSize() << endl << endl;
	delete [] bufferContent;

	senderSocket->send(senderBuffer);

	cout << "Buffer sent." << endl << endl;

	delete senderBuffer;

	senderSocket->send("A Sample Text");

	cout << "\"A Sample Text\" string is sent." << endl << endl;

	try
	{
		Socket *receiverSocket = acceptingSocket->accept();
		Buffer *receiverBuffer = receiverSocket->receive(14 + sizeof(int) + sizeof(char) + sizeof(long) + 2);

		cout << "Received Buffer Size: " << receiverBuffer->getSize() << endl;
		cout << "Received elements:" << endl;


		cout << (receiverBuffer->getString()).c_str() << endl;
		cout << receiverBuffer->getInt() << endl;
		cout << receiverBuffer->getChar() << endl;
		cout << receiverBuffer->getLong() << endl;
		cout << (int)receiverBuffer->getChar() << endl;
		cout << (int)receiverBuffer->getChar() << endl;

		receiverBuffer->resetCursor();
		cout << "resetCursor() is called." << endl;

		cout << (receiverBuffer->getString()).c_str() << endl;
		cout << receiverBuffer->getInt() << endl;
		cout << receiverBuffer->getChar() << endl;
		cout << receiverBuffer->getLong() << endl;
		cout << (int)receiverBuffer->getChar() << endl;
		cout << (int)receiverBuffer->getChar() << endl;

		//cout << receiverBuffer->getChar() << endl; // Exception

		delete receiverBuffer;

		string message = receiverSocket->getString();
		cout << "Received String: " <<  message.c_str() << endl;

		delete receiverSocket;
	}
	catch (Exception *e)
	{
		cout << e->getMessage() << endl;
		delete e;
	}

	delete senderSocket;
	delete acceptingSocket;

	return 0;
}
*/

//-------------------------------------------------------------------------------

#endif // AMIR_MOHSEN_SOCKET_CPP
