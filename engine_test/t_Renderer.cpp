#include "tests.hpp"
#include "application.hpp"

void T_Renderer()
{
	if( !Application::instance()->init() )
	{
		exit( -1 );
	}

	Application::instance()->run();
}