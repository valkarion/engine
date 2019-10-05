#include <Windows.h>
#include "application.hpp"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	if ( !Application::instance()->init() )
	{
		return -1;
	}

	Application::instance()->run();

	return 0;
}