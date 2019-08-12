#include <Windows.h>
#include "application.hpp"
#include "luaStateController.hpp"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd
)
{
	if ( Application::instance()->init() )
	{
		Application::instance()->run();
	}
		
	return 0;
}