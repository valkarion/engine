#define NOMINMAX				// supress windows.h min and max macros
#define WIN32_LEAN_AND_MEAN		// don't include unnecessary stuff 
#include <Windows.h>

#include "cvar.hpp"
#include "frameCounter.hpp"
#include "application.hpp"
#include "cvarSystem.hpp"
#include "loggers.hpp"
#include "fileSystem.hpp"
#include "renderer.hpp"

CVar window_width(			"window_width",			"1440" );
CVar window_height(			"window_height",		"900" );
CVar window_title(			"window_title",			"No Name Engine" );

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

	if( window == nullptr )
	{
		WriteToErrorLog( "Failed to create window." );
		return false;
	}

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

	Renderer::instance()->init();

	return true;
}

void SetWindowDebugTitle( GLFWwindow* window, int frameRate )
{
	char name[64];
	sprintf_s( name, 64, "%s | %d", 
		window_title.value.c_str(), frameRate );

	glfwSetWindowTitle( window, name );
}

void Application::run()
{
	FrameCounter frameCounter;

	while( !glfwWindowShouldClose( window ) )
	{
		glfwPollEvents();

		SetWindowDebugTitle( window, frameCounter.getFramerate() );
		frameCounter.update();
	}
}

void Application::shutdown()
{
	Renderer::instance()->stutdown();

	glfwDestroyWindow( window );
	glfwTerminate();
}