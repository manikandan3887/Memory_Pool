#include "memory_pool_container.h"

class MemoryObject
{

     public:

		MemoryObject( int a, int b);

	        void* operator new(size_t n)
	        {
		    return Container::getInstance()->allocateMem();
	        }

	        void operator delete(void * p)
	        {
		    Container::getInstance()->deallocateMem(p);
	        }

		int getAVal();

		int getBVal();

		void setAVal( int a);

		void setBVal( int a);
		
      private: //data
		int A;

		int B;

};

