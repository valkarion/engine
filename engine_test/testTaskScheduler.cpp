#include <boost/test/unit_test.hpp>
#include "taskScheduler.hpp"

BOOST_AUTO_TEST_SUITE( TaskSchedulerTests )

class NumberDoubler : public RangedTask
{
public:
	std::vector<int> numbers;
	std::vector<int> output;

	void init()
	{
		numbers.resize( 1'000'000 );		
		std::generate( numbers.begin(), numbers.end(), [i = 0]() mutable{
				return i++;
			} );

		output.resize( 1'000'000 );

		rangeSize = 1'000'000;
	}

	void execute( const TaskRange& range )
	{
		for ( size_t i = range.start; i < range.end; ++i )
		{
			output[i] = numbers[i] + numbers[i];
		}
	}
};

BOOST_AUTO_TEST_CASE( a_million_doubles )
{
	// Arrange 
	TaskScheduler* ts = TaskScheduler::instance();
	ts->initialize();

	std::unique_ptr<NumberDoubler> nd = std::make_unique<NumberDoubler>();
	nd->init();

	// Act 
	ts->execute( nd.get() );
	ts->waitFor( nd.get() );

	// Assert 
	bool valid = true;

	for ( int i = 0; i < nd->numbers.size(); i++ )
	{
		if ( nd->output[i] != ( 2 * nd->numbers[i] ) )
		{
			valid = false;
			break;
		}
	}

	BOOST_TEST( valid == true );

	ts->shutdown();
}

BOOST_AUTO_TEST_SUITE_END()