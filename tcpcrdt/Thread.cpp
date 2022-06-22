/*
Developed by:
	Mohsen Lesani
	Amir Moghimi
This program is under GNU GPL (for more information see http://www.gnu.org).
*/

#ifndef AMIR_MOHSEN_THREAD_CPP
#define AMIR_MOHSEN_THREAD_CPP

#include <pthread.h>

namespace amirmohsen
{
	/*
	* Usage:
	* Inherit a class from class Thread, override run() method and write your thread code in it.
	* Call start() to fire the thread.
	* Do not forget to link with -lpthread option.
	*/
	class Thread
	{
	private:
		pthread_t thread;
		pthread_attr_t attr;

		static void *runCaller(void * thread)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
			((Thread *)thread)->run();
		}

	public:
		// exit() terminates the execution of the calling thread.
		static void exit(int exitCode = 0)
		{
			pthread_exit(&exitCode);
		}

		virtual void run() = 0;

		Thread()          { pthread_attr_init(&attr); }
		virtual ~Thread() { pthread_cancel(thread); }

		void start()        { pthread_create(&thread, &attr, Thread::runCaller, this); }
		void waitToFinish() { pthread_join(thread, NULL); }
		/*
		* Suspends the execution of the calling thread until the
 		* thread terminates, either by calling exit() or by being
		* cancelled by finish() from another thread.
		*/
		void finish()       { pthread_cancel(thread); }
		/*
		* finish() is the mechanism by which a thread can terminate the
		* execution of another thread. Depending on its settings, the
		* target thread can then either ignore the request, honor it
		* immediately, or defer it till it reaches a cancellation point.
		*/
	};
}

//-------------------------------------------------------------------------------

//Test Thread
/*
using namespace amirmohsen;

int main(int argc, char* argv[])
{

}
*/

//-------------------------------------------------------------------------------

#endif // AMIR_MOHSEN_THREAD_CPP
