#pragma once

#include <memory>
#include <string>
#include "libs/sol.hpp"

template<typename T>
T solObjectToId( const sol::object& obj )
{
	T result = {};

	if ( obj.is<T>() )
	{
		result = obj.as<T>();
	}
	else
	{
		result = obj.as<int>();
	}

	return result;
}

class LuaStateController
{
	static std::unique_ptr<LuaStateController> _instance;
public:
	sol::state state;

	sol::protected_function_result safeRunScript( const std::string& script );
	sol::protected_function_result safeRunScriptFile( const std::string& file );

	void registerClasses(); // @luaClasses.cpp
	void registerFunctions(); // @luaFunctions.cpp 

	sol::table getDataTable( const std::string& accessor );

	static LuaStateController* instance();
};
