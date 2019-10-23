#include <boost/test/unit_test.hpp>
#include "luaStateController.hpp"

BOOST_AUTO_TEST_SUITE( LuaStateControllerTests )

// check on string script executions 
BOOST_AUTO_TEST_CASE( script_runner )
{
	sol::protected_function_result res;
	
	// check for failure but no crashing 
	res = LuaStateController::instance()->safeRunScript( "asd" );
	BOOST_TEST( res.valid() == false );

	// check for valid run 
	res = LuaStateController::instance()->safeRunScript( "return 2 + 2" );
	BOOST_TEST( res.valid() == true );
}

// check for script file loading 
BOOST_AUTO_TEST_CASE( script_file_runner )
{
	sol::protected_function_result res;

	// check for failure but no crashing 
	res = LuaStateController::instance()->safeRunScriptFile( "my_reasons_to_live.lua" );
	BOOST_TEST( res.valid() == false );

	// check for valid run 
	res = LuaStateController::instance()->safeRunScriptFile( "testRun.lua" );
	BOOST_TEST( res.valid() == true );
}

BOOST_AUTO_TEST_SUITE_END()