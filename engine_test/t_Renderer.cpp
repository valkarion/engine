#include "tests.hpp"
#include "application.hpp"
#include "inputSystem.hpp"
#include "luaStateController.hpp"
#include "camera.hpp"
#include <glm/glm.hpp>

void AddInputCommands()
{
	LuaStateController::instance()->safeRunScriptFile( "scripts\\data.lua" );
	LuaStateController::instance()->safeRunScriptFile( "scripts\\input.lua" );
	
	sol::table input = LuaStateController::instance()->getDataTable( "input" );
	sol::table keymap = LuaStateController::instance()->getDataTable( "keymap" );

	InputSystem::instance()->setupInputFunctions( input );
	InputSystem::instance()->setupKeyboardCommands( keymap );
}

void T_Renderer()
{
	if( !Application::instance()->init() )
	{		
		exit( -1 );
	}

	Camera::instance()->initCamera();
	AddInputCommands();

	Application::instance()->run();
}