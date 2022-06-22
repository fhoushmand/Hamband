/*
Developed by:
	Mohsen Lesani
	Amir Moghimi
This program is under GNU GPL (for more information see http://www.gnu.org).
*/

#ifndef AMIR_MOHSEN_EXCEPTION_CPP
#define AMIR_MOHSEN_EXCEPTION_CPP

#include <exception>
#include <string.h>

namespace amirmohsen
{
	class Exception
	{
	private:
		char *message;
	public:
		Exception(std::string m)
		{
			message = new char[m.length() + 11 + 1];
			message[0] = 0;
			strcpy(message, "Exception");
			strcat(message, m.c_str());
		}

		virtual ~Exception()
		{
			delete [] message;
		}

		char* getMessage()
		{
			return message;
		}
	};
}

//-------------------------------------------------------------------------------

// Exception Test
/*
using namespace amirmohsen;

int main(int argc, char* argv[])
{

}
*/

//-------------------------------------------------------------------------------

#endif // AMIR_MOHSEN_EXCEPTION_CPP

