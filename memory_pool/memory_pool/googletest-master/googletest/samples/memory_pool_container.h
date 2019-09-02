#include "memory_pool.hpp"

class Container 
{

    public:
	static Container* getInstance( int capacity = -1 );

        // cleanup
        virtual ~Container() {}

        // operator 'new' and 'delete' overloaded to redirect any memory management,
        // in this case delegated to the memory pool
	void* allocateMem();
	void deallocateMem(void* p);
	static void emptyContainer();

    private:
	typedef MemPool<Container > MemoryPool;
	static MemoryPool _mem;

	int _capacity;

	Container( int capacity ){
		_capacity = capacity;
		_mem.init(capacity);	
	};

	Container( Container const& ) {}; // Copy constructor is private

	Container& operator=( Container const&){
		return *this;
	}; // Assignment operator is private

        // Create a singleton object for the container	
        static Container* _pInstance;

    private:// Methods
	
};

    
//Container::MemoryPool Container::mem; // instance of the static memory pool
