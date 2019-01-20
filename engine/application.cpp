#define NOMINMAX				// supress windows.h min and max macros
#define WIN32_LEAN_AND_MEAN		// don't include unnecessary stuff 
#include <Windows.h>

#include "cvar.hpp"
#include "frameCounter.hpp"
#include "application.hpp"
#include "cvarSystem.hpp"

CVar window_width(	"window_width",		"800" );
CVar window_height( "window_height",	"600" );

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

std::unique_ptr<Application> Application::_instance = std::make_unique<Application>();

Application* Application::instance()
{
	return _instance.get();
}

bool Application::initGLFW()
{
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	
	window = glfwCreateWindow( window_width.intValue, 
		window_height.intValue, "Engine", nullptr, nullptr );

	// reposition the window to the center of the screen 
	const GLFWvidmode* videoMode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
	glfwSetWindowPos( window, videoMode->width / 2 - window_width.intValue / 2,
		videoMode->height / 2 - window_height.intValue / 2 );

	return true;
}

bool Application::init()
{
	CVarSystem::instance()->registerStaticCVars();

	initGLFW();

	return true;
}

void Application::run()
{
	FrameCounter frameCounter;

	while( !glfwWindowShouldClose( window ) )
	{
		glfwPollEvents();


		glfwSetWindowTitle( window, std::to_string( 
			frameCounter.getFramerate() ).c_str() );
		frameCounter.update();
	}
}

void Application::shutdown()
{
	glfwDestroyWindow( window );
	glfwTerminate();
}