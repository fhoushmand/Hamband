/*
Developed by:
	Mohsen Lesani
	Amir Moghimi
This program is under GNU GPL (for more information see http://www.gnu.org).
*/

#ifndef AMIR_MOHSEN_TIME_CPP
#define AMIR_MOHSEN_TIME_CPP

#include <iostream>
#include <sys/time.h>
#include "Socket.cpp"

namespace amirmohsen
{
	class Time
	{
	public:
		static long getTimeInMicros()
		{
			timeval tv;
			gettimeofday(&tv, NULL);
			return tv.tv_usec;
		}

		static long getTimeInSeconds()
		{
			timeval tv;
			gettimeofday(&tv, NULL);
			return tv.tv_sec;
		}

		static void delay(long seconds)
		{
			timeval tv;
			long elapsedSeconds = 0;
			gettimeofday(&tv, NULL);
			long startTime = tv.tv_sec;
			while (elapsedSeconds < seconds)
			{
				gettimeofday(&tv, NULL);
				elapsedSeconds = tv.tv_sec - startTime;
			}
		}
	};
}

//-------------------------------------------------------------------------------

//Time Test

using namespace amirmohsen;
using namespace std;

int main(int argc, char* argv[])
{

	int portNumber = 5777;
	if (argc > 1)
	{
		if (!(strcmp(argv[1],"-server")))
		{
			if (argc > 2)
			{
				if (!(portNumber = atoi(argv[2])))
				{
					cout << "Invalid port number!" << endl;
					exit(1);
				}
			}
			try
			{
				ServerSocket *server = new TCPServerSocket(portNumber);
				Socket *socket = server->accept();

				Buffer *buffer = socket->receive(sizeof(long));
				buffer->add(Time::getTimeInMicros());
				socket->send(buffer);

				delete buffer;
				delete socket;
				delete server;
			}
			catch(Exception *e)
			{
				cout << e->getMessage() << endl;
				delete e;
				exit(1);
			}

		}
		else if (!(strcmp(argv[1],"-client")))
		{
			if (argc > 3)
			{
				if (!(portNumber = atoi(argv[3])))
				{
					cout << "Invalid port number!" << endl;
					exit(1);
				}
			}
			if (argc > 2)
			{
				try
				{
					Socket *socket = new TCPSocket(argv[2], portNumber);

					Buffer *buffer = new Buffer(sizeof(long));
					buffer->add(Time::getTimeInMicros());
					socket->send(buffer);
					delete buffer;


					buffer = socket->receive(2 * sizeof(long));
					long startTime = buffer->getLong();
					long midTime = buffer->getLong();
					long currentTime = Time::getTimeInMicros();
					cout << "One way send delay = " << midTime - startTime << " us" << endl;
					cout << "One way receive delay = " << currentTime - midTime << " us" << endl;
					cout << "Round trip time = " << currentTime - startTime << " us" << endl;

					delete buffer;
					delete socket;

				}
				catch(Exception *e)
				{
					cout << e->getMessage() << endl;
					delete e;
					exit(1);
				}
			}
			else
			{
				cout << "Host name missed!" << endl;
				exit(1);
			}
		}
		else
		{
			cout << "Usage:" << endl << "time -server [port number]" << endl
			     << "time -client host [port number]" << endl;

		}
	}
	else
	{
		cout << "Usage:" << endl << "time -server [port number]" << endl
		     << "time -client host [port number]" << endl;
	}


}

//-------------------------------------------------------------------------------

#endif // AMIR_MOHSEN_TIME_CPP
