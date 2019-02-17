#include "tests.hpp"
#include <iostream>
#include <chrono>
#include <vector>

#include "taskScheduler.hpp"

const int test_elem_count = 10'000'000;
#define NOW std::chrono::high_resolution_clock::now()

class TestSystem final : public RangedTask
{
public:
	std::vector<float> thingsToProcess;

	void init()
	{
		thingsToProcess.clear();

		rangeSize = test_elem_count;
		for( size_t i = 0; i < test_elem_count; i++ )
		{
			thingsToProcess.push_back( (float)i );
		}
	}

	void update( int index )
	{
		thingsToProcess[index] = std::powf( thingsToProcess[index], 3 );
	}

	void execute( const TaskRange& range ) override final
	{
		for( size_t i = range.start; i < range.end; i++ )
		{
			update( i );
		}
	}
};

void T_TaskScheduler()
{
	TestSystem sys;
	sys.init();

	auto start = NOW;
	for( size_t i = 0; i < sys.thingsToProcess.size(); i++ )
	{
		sys.update( i );
	}
	auto end = NOW;
	auto delta_serial = std::chrono::duration_cast<std::chrono::milliseconds>( end - start );

	TaskScheduler* ts = TaskScheduler::instance();
	ts->initialize();
	sys.init();

	start = NOW;

	ts->execute( &sys );
	ts->waitFor( &sys );

	end = NOW;
	auto delta_parallel = std::chrono::duration_cast<std::chrono::milliseconds>( end - start );

	std::printf( "Serial:\t\t %lld", delta_serial.count() );
	std::printf( "\nParallel:\t %lld", delta_parallel.count() );
	
	std::cin.get();

	ts->shutdown();
}