#include <Windows.h>
#include "application.hpp"


int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow )
{
	if( !Application::instance()->init() )
	{
		return -1;
	}

	Application::instance()->run();

	return 0;
}
