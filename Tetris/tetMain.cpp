#include <Windows.h>
#include "application.hpp"
#include "tetRenderer.hpp"
#include "luaStateController.hpp"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	Renderer::instance()->setInstanceType<TetRenderer>();
		
	if ( !Application::instance()->init() )
	{
		return -1;
	}

	LuaStateController::instance()->safeRunScriptFile( "startup.lua" );

	Application::instance()->run();

	return 0;
}