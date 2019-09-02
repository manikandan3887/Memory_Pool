#ifndef __MEMPOOL_STD_HEAP__HPP__
#define __MEMPOOL_STD_HEAP__HPP__

#include<iostream>
#include<algorithm>
#include<stdint.h>
#include <memory>
#include<pthread.h>
//#include<mutex>


//pthread_mutex_t lock;

using namespace std;

template <typename T> class MemPool 
{
    private:
        // states of the heap nodes
        enum State { FREE = 1, TAKEN = 0 };

        struct Entry {
            State state; // the state of the memory chunk
            T * p; // pointer to the memory chunk

            // comparsion operator, needed for heap book keeping
            bool operator<(const Entry & other) const
            { return state < other.state; }
        };

        typedef unsigned int size_type; // convenience
    private:
        size_type available; // number of memory chunks available
	int N;
        //Entry entry[N]; // book keeping
        //uint8_t buf[sizeof(T)*N]; // the actual memory, here will the objects be stored
        Entry* entry; // book keeping
        uint8_t* buf; // the actual memory, here will the objects be stored
    public:
        void init( int n )
        {
            // number of available memory chunks is the size of the memory pool
		
	    N = n;	
            available = N;

	    //cout << "Available value is: " << available << endl;

	    //buf = ( uint8_t*) malloc( sizeof(T) * N );

	    buf = new uint8_t[ sizeof(T)*N ];

	    entry = new Entry[N];

            // all memory chunks are free, pointers are initialized
            for (size_type i = 0; i < N; ++i) {
                entry[i].state = FREE;
                entry[i].p = reinterpret_cast<T *>(&buf[sizeof(T)*i]);
            }

            // make heap of book keeping array
            make_heap(entry, entry+N);
        }

        void * allocate()
        {
            // allocation not possible if memory pool has no more space
            if (available <= 0 || available > N)  throw bad_alloc();
	
	    //mtx.lock();
	    //pthread_mutex_lock(&lock);

            // the first memory chunk is always on index 0
            Entry e = entry[0];

            // remove first entry from heap
            pop_heap(entry, entry+N);

            // one memory chunk allocated, no more available
            --available;

            // mark the removed chunk
            entry[available].state = TAKEN;
            entry[available].p = NULL;

	    //mtx.unlock();
	    //pthread_mutex_unlock(&lock);

            // return pointer to the allocated memory
            return e.p;
        }

        void deallocate(void * ptr)
        {
            // invalid pointers are ignored
            if (!ptr || available >= N) return;
	    //pthread_mutex_lock(&lock);
	    //mtx.lock();

            // mark freed memory as such
            entry[available].state = FREE;
            entry[available].p = reinterpret_cast<T *>(ptr);

            // freed memory chunk, one more available
            ++available;

            // heap book keeping
            push_heap(entry, entry+N);
	    //mtx.unlock();
	    //pthread_mutex_unlock(&lock);
        }
};

#endif
