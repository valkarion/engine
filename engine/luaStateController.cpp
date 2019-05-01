#include "luaStateController.hpp"
#include "loggers.hpp"

std::unique_ptr<LuaStateController> LuaStateController::_instance = std::make_unique<LuaStateController>();
LuaStateController* LuaStateController::instance()
{
	return _instance.get();
}

sol::protected_function_result LuaStateController::safeRunScript( const std::string& script )
{
	return state.safe_script( script,
		[this]( lua_State*, sol::protected_function_result pfr )
		{
			sol::error err = pfr;
			PrintToOutputWindow( err.what() );
			return pfr;
		} );
}

sol::protected_function_result LuaStateController::safeRunScriptFile( const std::string& file )
{
	return state.safe_script_file( file,
		[this]( lua_State*, sol::protected_function_result pfr )
		{
			sol::error err = pfr;
			PrintToOutputWindow( err.what() );
			return pfr;
		} );
}

sol::table LuaStateController::getDataTable( const std::string& accessor ) const
{
	sol::table d = state["data"];
	if ( d != sol::nil )
	{		
		if ( d[accessor] != sol::nil )
		{
			return d[accessor];
		}

		WriteToErrorLog( "Failed to get datatable with name: %s", accessor.c_str() );
		return sol::nil;
	}

	return sol::nil;
}