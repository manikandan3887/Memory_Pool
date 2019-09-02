#include "memory_pool_mem_object.h"

MemoryObject::MemoryObject( int a, int b )
{
	A = a, B = b;
}

int MemoryObject::getAVal(){
	return A;
}
int MemoryObject::getBVal(){
	return B;
}
void MemoryObject::setAVal( int a){
	A = a;
}

void MemoryObject::setBVal( int b ){
	B = b;
}
