#include "luaStateController.hpp"

std::unique_ptr<LuaStateController> luaStateController = std::make_unique<LuaStateController>();

sol::protected_function_result LuaStateController::safeRunScript( const std::string& script )
{
	return state.safe_script( script,
		[this]( lua_State*, sol::protected_function_result pfr )
		{
			sol::error err = pfr;
			return pfr;
		} );
}

sol::protected_function_result LuaStateController::safeRunScriptFile( const std::string& file )
{
	return state.safe_script_file( file,
		[this]( lua_State*, sol::protected_function_result pfr )
		{
			sol::error err = pfr;
			return pfr;
		} );
}