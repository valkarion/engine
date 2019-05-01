#pragma once

#include <memory>
#include <string>
#include "libs/sol.hpp"

class LuaStateController
{
	static std::unique_ptr<LuaStateController> _instance;
public:
	sol::state state;

	sol::protected_function_result safeRunScript( const std::string& script );
	sol::protected_function_result safeRunScriptFile( const std::string& file );

	void registerClasses(); // @luaClasses.cpp
	void registerFunctions(); // @luaFunctions.cpp 

	sol::table getDataTable( const std::string& accessor ) const;

	static LuaStateController* instance();
};
