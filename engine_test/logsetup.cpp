#include <boost/test/unit_test.hpp>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

/*
	Based on this: https://stackoverflow.com/a/33755797
	last viewed: 2019.10.23
*/
class VSLogger : public std::stringbuf
{
	std::ostream self;
public:
	VSLogger() : self( this )
	{
		boost::unit_test::unit_test_log.set_stream( self );
	}

	~VSLogger()
	{
		sync(); // can be avoided
		boost::unit_test::unit_test_log.set_stream( std::cout );
	}

	int sync()
	{
		OutputDebugString( str().c_str() );
		str( "" );
		return 0;
	}
};

/*
	global fixutes are attached to the master suite and exec'd before testing
*/
BOOST_TEST_GLOBAL_CONFIGURATION( VSLogger );