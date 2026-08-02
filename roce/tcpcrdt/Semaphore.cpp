/*
Developed by:
	Mohsen Lesani
	Amir Moghimi
This program is under GNU GPL (for more information see http://www.gnu.org).
*/

#ifndef AMIR_MOHSEN_SEMAPHORE_CPP
#define AMIR_MOHSEN_SEMAPHORE_CPP

#include <semaphore.h>
#include <iostream>

namespace amirmohsen
{

	class Semaphore
	{
	private:
		sem_t s;

	public:
		Semaphore(int value = 1) { sem_init(&s, 0, value); }
		~Semaphore()             { sem_destroy(&s); }

		void wait()    { sem_wait(&s); }
		void signal()  { sem_post(&s); }
		int getValue() { int val; sem_getvalue(&s, &val); return val; }
	};

}

//-------------------------------------------------------------------------------


//Test Semaphore
/*
using namespace amirmohsen;
using namespace std;

int main(int argc, char* argv[])
{
	Semaphore testSem;
	cout << "Default value: " << testSem.getValue() << "\n";
	testSem.wait();
	cout << "Value after wait(): " << testSem.getValue() << "\n";
	testSem.signal();
	cout << "Value after one signal() call: " <<  testSem.getValue() << "\n";
	testSem.signal();
	testSem.signal();
	cout << "Value after two signal() calls: " <<  testSem.getValue() << "\n";
	return 0;
}
*/

//-------------------------------------------------------------------------------

#endif // AMIR_MOHSEN_SEMAPHORE_CPP
