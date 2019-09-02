#include "memory_pool_container.h"

pthread_mutex_t lock;
Container* Container::_pInstance = NULL;

Container* Container::getInstance( int capacity )
{
	if( capacity != -1 )
	{
		if( ! _pInstance )
			_pInstance = new Container( capacity ) ;
	}
	return _pInstance;
}

void Container::emptyContainer()
{
	delete _pInstance;
	_pInstance = NULL;
}

void* Container::allocateMem()
{
//	pthread_mutex_lock(&lock);
	//mtx.lock();
	return _mem.allocate();
//	pthread_mutex_unlock(&lock);
	//mtx.unlock();
}

void Container::deallocateMem(void* p)
{
	pthread_mutex_lock(&lock);
	//mtx.lock();
	_mem.deallocate(p);
	pthread_mutex_unlock(&lock);
	//mtx.unlock();
}

