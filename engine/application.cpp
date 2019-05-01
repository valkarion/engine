#include <Windows.h>

#include "cvar.hpp"
#include "frameCounter.hpp"
#include "application.hpp"
#include "cvarSystem.hpp"
#include "loggers.hpp"
#include "fileSystem.hpp"
#include "renderer.hpp"
#include "inputSystem.hpp"
#include "resourceManager.hpp"
#include "luaStateController.hpp"

CVar window_width(			"window_width",			"1280" );
CVar window_height(			"window_height",		"800" );
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
	
	Renderer* r = Renderer::instance();

	r->window = glfwCreateWindow( window_width.intValue,
		window_height.intValue, "Engine", nullptr, nullptr );

	if( r->window == nullptr )
	{
		WriteToErrorLog( "Failed to create window." );
		return false;
	}

	// reposition the window to the center of the screen 
	const GLFWvidmode* videoMode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
	glfwSetWindowPos( r->window, videoMode->width / 2 - window_width.intValue / 2,
		videoMode->height / 2 - window_height.intValue / 2 );

	// set the cursor to the center of the screen 
	glfwSetCursorPos( r->window, window_width.floatValue / 2.f, 
		window_height.floatValue / 2.f );

	// disable cursor 
	glfwSetInputMode( r->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

	return true;
}

bool Application::init()
{
	CVarSystem::instance()->registerStaticCVars();

	LuaStateController::instance()->state.open_libraries(
		sol::lib::base,	sol::lib::package, sol::lib::string,
		sol::lib::table, sol::lib::math );
	LuaStateController::instance()->registerFunctions();
	LuaStateController::instance()->registerClasses();

	initGLFW();

	Renderer::instance()->init();
	
	InputSystem::instance()->init( Renderer::instance()->window );

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

	while( !exitGame && !glfwWindowShouldClose( Renderer::instance()->window ) )
	{
		glfwPollEvents();

		InputSystem::instance()->update();

		Renderer::instance()->drawFrame();

		SetWindowDebugTitle( Renderer::instance()->window, frameCounter.getFramerate() );
		frameCounter.update();
	}

	shutdown();
}

void Application::quit()
{
	exitGame = true;
};

void Application::shutdown()
{
	Renderer::instance()->shutdown();

	ResourceManager::instance()->shutdown();

	glfwDestroyWindow( Renderer::instance()->window );
	glfwTerminate();
}