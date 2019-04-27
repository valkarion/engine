#include "tests.hpp"
#include "luaStateController.hpp"

// test function to check Lua/C++ function bindings
int Calculate( int a, int b )
{
	return a + b;
}

// test class to check Lua/C++ class bindings 
class TestClass
{
	int myVariable;
public:
	int getMyVariable() const
	{
		return myVariable;
	}

	void setMyVariable( const int variable )
	{
		if ( variable < 0 )
		{
			std::printf( "Tried to set var to a negative value\n" );
			myVariable = 0;
		}
		else
		{
			printf( "Value set\n" );
			myVariable = variable;
		}
	}

	void memberFunction() const
	{
		std::printf( "Member Function Called!\n" );
	}

	void memberFunction( const int ) const
	{
		std::printf( "Overloaded Member Function Called!\n" );
	}

	TestClass() {}
	TestClass( const int n ) : myVariable( n ) {}
	~TestClass()
	{
		std::printf( "Destructor Called\n" );
	}
};

void T_LuaController()
{	
// check valid script run 
	auto res = luaStateController->safeRunScript( "return 2 + 2" );	
	assert( res.status() == sol::call_status::ok );

	std::printf( "Valid script run OK\n" );

// check invalid script run 
	res = luaStateController->safeRunScript( " 3 + \"Alma\"" );
	assert( res.status() != sol::call_status::ok );

	std::printf( "Invalid script exception caught OK\n" );

// register and test a C++ function
	luaStateController->state["Calculate"] = Calculate;
	res = luaStateController->safeRunScript( "return Calculate(2, 3)" );
	int a = res.get<int>();
	assert( res.status() == sol::call_status::ok && a == 5 );
	
	std::printf( "Lua/C++ function bidning OK\n" );

// register and test C++ class 
	luaStateController->state.new_usertype<TestClass>( "TestClass",
		"myVar", sol::property( &TestClass::getMyVariable, &TestClass::setMyVariable ),
		"memFn", sol::overload(
			sol::resolve<void() const>( &TestClass::memberFunction ),
			sol::resolve<void( const int ) const>( &TestClass::memberFunction ) ),
		"new", sol::constructors<TestClass(), TestClass( const int )>()
		);

	assert( luaStateController->state["TestClass"] != sol::nil );

	std::printf( "C++ class was registered to lua OK\n" );

	res = luaStateController->safeRunScript( R"(
		local test = TestClass:new(2)
		test.myVar = 5
		test.myVar = -2
		test:memFn()
		test:memFn(3)
		test = nil
)" );

	assert( res.status() == sol::call_status::ok );
}