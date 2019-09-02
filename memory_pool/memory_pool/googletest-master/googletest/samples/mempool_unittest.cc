#include <stdio.h>
#include <stdlib.h>
#include "memory_pool_mem_object.h"
#include "gtest/gtest.h"
#include <thread>
#include <mutex>
#include <future>
#include "ThreadPool.h"
#include <unistd.h>
#include <fstream>
#include "macrologger.h"

mutex m;
using nbsdx::concurrent::ThreadPool;
//#include <boost/filesystem.hpp>

Container::MemoryPool Container::_mem; // instance of the static memory pool
using namespace std;

	class MemoryPoolTest: public ::testing::Test {
	public:
		enum Operations{
			ADD_OPERATION,
			DELETE_OPERATION,
			UPDATE_OPERATION,
			SUM
		};
		MemoryPoolTest(){
		    //LOG_DEBUG ( "In constructor" );
		}

		void setCapacity( int capacity ){
			LOG_DEBUG("The capacity to be set in container is: %d", capacity);			
			Container::getInstance(capacity);
		    	_capacity = capacity;
		}

		void addMemObjects ( int num ){
		//	bool flag = false;
			try{

				//LOG_DEBUG ( "Allocating mem objects: " << num );
				int lastObjValue = _memObjects.size();
	    			for (int i = 0; i < num; i++){
					 //LOG_DEBUG ( "Allocating Memobject for index: " << i );
					 m.lock();
					 usleep(300);
					 //std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
					 _memObjects.push_back( new MemoryObject( lastObjValue , lastObjValue +1 ) );	
					 m.unlock();
					 lastObjValue++;
				}
		//		flag = true;
			}
			catch(bad_alloc& ba ){
				LOG_ERROR("Bad alloc caught: %s ", ba.what() );	
		//		flag = false;
			}
		//	return flag;
		}

		void updateMemObject( int index, int aValue, int bValue )
		{
			try{

				//LOG_DEBUG ( "Updating memory object. " );
				if ( index > _capacity ){
					
					LOG_ERROR("Index is greater than capacity. The object cannot be updated ");
					return;
				}	
				//_memObjects.push_back( new MemoryObject( aValue, bValue ) );
				if ( index >= (int) _memObjects.size() ){
					 _memObjects.resize(index);
				}
				usleep(400);
				if( _memObjects[ index - 1] == NULL ){
					m.lock();
					//LOG_DEBUG ( "Creating the object here. " );
					_memObjects[ index  - 1 ] = new MemoryObject( aValue, bValue );
					m.unlock();
				}
				else{
					_memObjects[index - 1]->setAVal( aValue );
					_memObjects[index - 1]->setBVal( bValue );
				}
			}
			catch(bad_alloc& ba ){
				LOG_ERROR(" Bad alloc caught: %s", ba.what() );	
			}
		}
	

		void retrieveMemObjectContents( int index, int* arr ){
			//LOG_DEBUG ( "Mem object size is: " << _memObjects.size() << "Index is: " << index );
			if(index > _capacity ){
				LOG_ERROR( "Object is not present in the container " );
				return;
			}
			else if( isNullObject( index ) ){
				LOG_ERROR("Object is NULL. ");
			}
			else
			{
				arr[0] = _memObjects[index - 1]->getAVal();
				arr[1] = _memObjects[index - 1]->getBVal();
			}
		}
				
		void deleteMemObjects(){
	    		//delete objects, free memory
	    		try{

		    		//LOG_DEBUG ( "Deleting all the Mem objects: " );
				for( unsigned int iter = 0; iter != _memObjects.size(); ++iter)
				{
	    				m.lock();
	  			  	delete _memObjects[iter];
	     				_memObjects[iter] = NULL;
					m.unlock();
      		   		 }
				_memObjects.clear();
			}
			catch(bad_alloc& ba ){
				LOG_ERROR("Bad alloc caught: %s", ba.what() );	
			}
		}
		
		void deleteDiffMemObjects( vector<int> nums ){
			LOG_DEBUG("Deleting Mem Objects");
			for( unsigned int i = 0; i < nums.size(); i++ )
			{
				if ( nums[i] > _capacity ){
					LOG_ERROR ( "Cannot remove this index as the container size is lesser than this index " );
					continue;
				}
				if ( isNullObject( nums[i]  ) )
				{
					LOG_ERROR ("The objects is already NULL " );
					continue;
				}
				usleep(500);
				delete _memObjects[ nums[i] - 1 ];
				_memObjects[ nums[i] - 1 ] = NULL;
			}
		}

		void threadFunc( vector<int>& values, Operations op, int noofthreads ){
			//LOG_DEBUG ("Executing the operation: " (op );
			vector<thread> threadsVect;
			threadsVect.reserve( noofthreads );
			if ( op == ADD_OPERATION ){
				 int no_of_objects_to_be_added_per_thread = values[0] / noofthreads ; 
				 for( int i = 0; i < noofthreads; i++ ){
					threadsVect.emplace_back( &MemoryPoolTest::addMemObjects, this, no_of_objects_to_be_added_per_thread );
				   }
			}
			else if ( op == DELETE_OPERATION ){
				 //LOG_DEBUG ("Deleting mem objects" );
				   for( int i = 0; i < noofthreads; i++ ){
					threadsVect.emplace_back( &MemoryPoolTest::deleteDiffMemObjects, this, ref( values ) );
				   }
			}

			for( auto& t: threadsVect )
				t.join();	
		} 
	
		~MemoryPoolTest(){
			//delete[] memObject;
		}

		// Setting up 
		void SetUp(int capacity ) 
		{
			LOG_DEBUG("");			
			LOG_DEBUG("---------- Starting to execute TEST CASE --------");
			_capacity = capacity;
			setCapacity(_capacity);
		}	

		bool isNullObject( unsigned int index )
		{
			return index <= 0 || index > _memObjects.size() ||  _memObjects[index - 1] == NULL;
		}

		int sum( int index )
		{
			return _memObjects[index-1]->getAVal() + _memObjects[index-1]->getBVal();
		}	
		
		// In the end of every test case, initializing the container Capacity to 0
		// deleting the memory Objects references
		void TearDown() override
		{
			deleteMemObjects();
			Container::emptyContainer();
			_capacity = 0;
			_numOfMemObjectReferencesCreated = 0;
			LOG_INFO("----------------------End of TEST CASE ---------------------------\n");
		}

	private: // variables
		int _capacity;
	    	vector<MemoryObject*> _memObjects;
		int _numOfMemObjectReferencesCreated;
	};

	
	TEST_F( MemoryPoolTest , without_thread_get_object_test_case1 ){
		SetUp( 4 );
		LOG_INFO(" TEST CASE 1: Without thread object retrieval 1");
		LOG_DEBUG("Updating 1st object with {12,23} and 2nd object with contents {22,35}");
		updateMemObject(1,12,23);
		updateMemObject( 2,22,35);
		int expected[2] = {0};
		retrieveMemObjectContents( 1, expected );
	 	int comp[2] = {12,23};
		EXPECT_TRUE( 0 == memcmp( expected , comp, 2) );
		retrieveMemObjectContents( 2, expected );
		LOG_DEBUG( "Comparing the 2nd object with the contents {12,23}" ); 
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
	}

	TEST_F( MemoryPoolTest , without_thread_get_object_boundary_case1 ){
		SetUp(4);
		LOG_INFO ("TEST CASE 2: Without thread object retrieval 2" );
		LOG_DEBUG ("Container capacity: 4. Comparing the 4th object with the contents {33,12}" );
		updateMemObject( 4, 33,12 );
		int expected[2] = {0};
		retrieveMemObjectContents( 4, expected );
	 	int comp[2] = {33,12};
		EXPECT_TRUE( 0 == memcmp( expected , comp, 2) );
	}
	
	TEST_F( MemoryPoolTest , without_thread_get_object_boundary_case2 ){
		SetUp(4);
		LOG_INFO ("TEST CASE 3: Without thread object retrieval 3" );
		updateMemObject( 5, 33,12 );
		int expected[2] = {0};
		retrieveMemObjectContents( 5, expected );
	 	int comp[2] = {33,12};
		LOG_DEBUG ("Container capacity: 4. Comparing the 5th object with the contents {33,12}" );
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
	}	

	TEST_F( MemoryPoolTest , without_thread_with_delete ){
		SetUp(4);
		LOG_INFO ("TEST CASE 4: Without thread with delete" );
		updateMemObject( 1, 12, 23 );
		updateMemObject( 2, 22, 33 );
		updateMemObject( 4, 33,12 );
		int expected[2] = {0};
		LOG_DEBUG ("Updated 3 objects(1,2,4)." );
		LOG_DEBUG ("EXPECT FALSE: Obj[4] = {33,12} with {12,33}" );
		retrieveMemObjectContents( 4, expected );
	 	int comp[2] = {12,23};
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
		vector<int> toDel;
		toDel.push_back(1);
		LOG_DEBUG ("Deleting 1st memory object" );
		deleteDiffMemObjects(toDel);
		LOG_DEBUG ("Retrieving 1st memory object" );
		retrieveMemObjectContents( 1, expected );
	 	comp[0] = 12, comp[1] = 23;
		LOG_DEBUG ("Comparing the memory object: {12,23} " );
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
		retrieveMemObjectContents( 2, expected );
	 	comp[0] = 22, comp[1] = 33;
		LOG_DEBUG ("MemObject to be compared: {22,33}, toComp:{22,33}" );
		EXPECT_TRUE( 0 == memcmp( expected , comp, 2) );
	}

	TEST_F( MemoryPoolTest , without_thread_with_delete_boundary_test_case ){
		SetUp(4);
		LOG_INFO ("TEST CASE 5: Without thread delete object boundary test case" );
		updateMemObject( 4, 33,12 );
		int expected[2] = {0};
		retrieveMemObjectContents( 4, expected );
	 	int comp[2] = {12,23};
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
		vector<int> toDel;
		toDel.push_back(4);
		deleteDiffMemObjects(toDel);
		fill_n(expected,2,0);
		retrieveMemObjectContents( 4, expected );
	 	comp[0] = 33, comp[1] = 12;
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
		retrieveMemObjectContents( 2, expected );
	 	comp[0] = 22, comp[1] = 33;
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
	}

	TEST_F( MemoryPoolTest , without_thread_with_delete_add_object_boundary_test_case ){
		SetUp(4);
		LOG_INFO ("TEST CASE 6: Without thread delete and add object boundary test case" );
		updateMemObject( 4, 33,12 );
		int expected[2] = {0};
		retrieveMemObjectContents( 4, expected );
	 	int comp[2] = {12,23};
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
		vector<int> toDel;
		toDel.push_back(4);
		deleteDiffMemObjects(toDel);
		fill_n(expected,2,0);
		retrieveMemObjectContents( 4, expected );
	 	comp[0] = 33, comp[1] = 12;
		EXPECT_FALSE( 0 == memcmp( expected , comp, 2) );
		updateMemObject( 4, 33,12 );
		retrieveMemObjectContents( 4, expected );
	 	comp[0] = 33, comp[1] = 12;
		EXPECT_TRUE( 0 == memcmp( expected , comp, 2) );
	}

	TEST_F( MemoryPoolTest , without_thread_accessing_null_objects ){
		SetUp( 10 );
		LOG_INFO ("TEST CASE 7: Without thread accessing null objects" );
		addMemObjects(4);
		EXPECT_EQ( sum(1),1 );
		EXPECT_EQ( sum(2),3);
		EXPECT_EQ( isNullObject(3), false ); 
		EXPECT_EQ( isNullObject(10), true ); 
		EXPECT_EQ( isNullObject(11), true ); 
	}
	
	TEST_F( MemoryPoolTest , without_thread_deleting_object_test_case2 ){
		SetUp( 10 );
		LOG_INFO ("TEST CASE 8: Without thread deleting object test case 2" );
		addMemObjects(4);
		EXPECT_EQ( isNullObject(3), false ); 
		vector<int> toDel;
		toDel.push_back(1);
		toDel.push_back(3);
		deleteDiffMemObjects( toDel );
		ASSERT_TRUE( isNullObject( 3) );
		addMemObjects( 1 );
		EXPECT_EQ( isNullObject(5), false ); 
		addMemObjects( 4 );
		EXPECT_EQ( isNullObject(10), true );
		toDel.clear();
		toDel.push_back( 9 );
		deleteDiffMemObjects( toDel );
		ASSERT_TRUE( isNullObject( 9 ) );
		EXPECT_NE( sum(8),20 );
		EXPECT_EQ( sum(8),15 );
	}

	TEST_F( MemoryPoolTest , with_thread_normal_condition ){
		SetUp( 10 );
		LOG_INFO ("TEST CASE 9: With thread normal condition 1" );
		thread t1( &MemoryPoolTest::addMemObjects, this, 5 );
		t1.join();
		EXPECT_EQ( isNullObject(3), false );
		EXPECT_EQ( isNullObject(7), true );
		thread t2( &MemoryPoolTest::addMemObjects, this, 5 );
		t2.join();
		EXPECT_EQ( isNullObject(7), false );
	}

	TEST_F( MemoryPoolTest , with_thread_normal_condition_2 ){
		SetUp( 10 );
		LOG_INFO ("TEST CASE 10: Thread Normal condition 2" );
		ThreadPool<3> pool; // Defaults to 10 threads.
  		int JOB_COUNT = 3;
		for( int i = 0; i < JOB_COUNT; ++i )
	        {
		       pool.AddJob( [this]() { addMemObjects(2); 
			});
		}
		pool.JoinAll();
		std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
		EXPECT_EQ( isNullObject(3), false );
		EXPECT_EQ( isNullObject(7), true );
	}

	TEST_F( MemoryPoolTest , with_thread_normal_condition_3 ){
		SetUp( 10 );
		LOG_INFO ("TEST CASE 11: Thread Normal condition 3" );
		ThreadPool<5> pool; // Defaults to 10 threads.
  		int JOB_COUNT = 5;
		for( int i = 0; i < JOB_COUNT; ++i )
	        {
		       pool.AddJob( [this]() { addMemObjects(2);
			});
		}
		pool.JoinAll();
		std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
		EXPECT_EQ( isNullObject(3), false );
		EXPECT_EQ( isNullObject(10), false );
		EXPECT_NE( sum(8),20 );
		updateMemObject( 8,4,11);
		EXPECT_EQ( sum(8),15 );
	}

	TEST_F( MemoryPoolTest , with_thread_boundary_condition_1 ){
		SetUp( 10 );
		LOG_INFO ("TEST CASE 12: With thread boundary condition 1" );
		ThreadPool<5> pool; // Defaults to 10 threads.
  		int JOB_COUNT = 2;
		for( int i = 0; i < JOB_COUNT; ++i )
	        {
		       pool.AddJob( [this]() { addMemObjects(2);
				//std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
			});
		}
		pool.JoinAll();
		std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
		EXPECT_EQ( isNullObject(3), false );
		EXPECT_EQ( isNullObject(10), true );
		EXPECT_NE( sum(1),20 );
		updateMemObject( 1,4,11);
		EXPECT_EQ( sum(1),15 );
		updateMemObject( 1,4,15);
		EXPECT_NE( sum(1),15);
	}

	TEST_F( MemoryPoolTest , with_thread_test_case_3 ){
		SetUp( 10 );
		LOG_INFO ( "TEST CASE 13: Thread Boundary condition 2" );
		int noofthreads = 2;
		vector<int> values = {10};
		threadFunc( values, ADD_OPERATION, noofthreads );
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
		values.clear();
		values = {1,3,5,7,9} ;
		threadFunc( values, DELETE_OPERATION,1 );
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
		EXPECT_NE( isNullObject(2), true ); 
		EXPECT_EQ( isNullObject(1), true ); 
	}

	TEST_F( MemoryPoolTest , without_thread_boundary_conditions_check_test_case2 ){
		EXPECT_EQ( isNullObject(1), true ); 
		SetUp( 10 );
		LOG_INFO ( "TEST CASE 14: Final test " );
	 	vector<int> toDel;
		toDel.push_back( 1 );
		deleteDiffMemObjects( toDel );	
		std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
		EXPECT_EQ( isNullObject(2), true ); 
		EXPECT_EQ( isNullObject(2), true );
		toDel.clear();
		addMemObjects(4);
		ASSERT_FALSE( isNullObject(4) );
		EXPECT_NE( sum(4), 6 );
		ASSERT_TRUE( isNullObject(5) );
	}


int main(int argc, char **argv)
{
    LOG_DEBUG ( "---- Memory pool created by Manikandan Balasubramanian ---------- ");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
