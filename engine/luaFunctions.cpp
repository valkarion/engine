#include "luaStateController.hpp"
#include "loggers.hpp"
#include "camera.hpp"
#include "cvar.hpp"
#include "cvarSystem.hpp"

extern CVar window_title;

// utils
void DebugPrint( const std::string& message )
{
	PrintToOutputWindow( message );
}

void SetWindowTitle( const std::string& title )
{
	window_title.setValue( title );
}

void SetCVar( const std::string& cvar, sol::object value )
{
	CVar* cv = CVarSystem::instance()->find( cvar );
	if ( cv )
	{
		cv->setValue( value.as<std::string>() );
	}
}

// objects 
Camera* GetCamera()
{
	return Camera::instance();
}

void LuaStateController::registerFunctions()
{
	state["DebugPrint"] = DebugPrint;
	state["SetWindowName"] = SetWindowTitle;
	state["SetCVar"] = SetCVar;

	state["GetCamera"] = GetCamera;
}