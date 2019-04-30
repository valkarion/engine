#include "tests.hpp"
#include "application.hpp"
#include "inputSystem.hpp"
#include "luaStateController.hpp"
#include "camera.hpp"
#include <glm/glm.hpp>

extern Camera camera;

void AddInputCommands()
{
	const float cameraSens = 0.01f;	
	InputSystem* is = InputSystem::instance();
	sol::state& l = LuaStateController::instance()->state;
}

void T_Renderer()
{
	if( !Application::instance()->init() )
	{		
		exit( -1 );
	}

	camera.initCamera();
	AddInputCommands();

	Application::instance()->run();
}