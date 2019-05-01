#include "luaStateController.hpp"
#include "loggers.hpp"
#include "camera.hpp"

// utils
void DebugPrint( const std::string& message )
{
	PrintToOutputWindow( message );
}

// objects 
Camera* GetCamera()
{
	return Camera::instance();
}

void LuaStateController::registerFunctions()
{
	state["DebugPrint"] = DebugPrint;

	state["GetCamera"] = GetCamera;
}