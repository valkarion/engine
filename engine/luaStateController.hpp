#pragma once

#include <memory>
#include <string>
#include "libs/sol.hpp"

class LuaStateController
{
public:
	sol::state state;

	sol::protected_function_result safeRunScript( const std::string& script );
	sol::protected_function_result safeRunScriptFile( const std::string& file );
};

extern std::unique_ptr<LuaStateController> luaStateController;