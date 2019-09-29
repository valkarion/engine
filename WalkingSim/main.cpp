#include <Windows.h>
#include "application.hpp"
#include "luaStateController.hpp"

#include "renderer.hpp"
#include "wsRenderer.hpp"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd
)
{
	Renderer::instance()->setInstanceType<WsRenderer>();

	if ( Application::instance()->init() )
	{
		LuaStateController::instance()->safeRunScriptFile( "startup.lua" );
		
		Application::instance()->run();
	}
		
	return 0;
}