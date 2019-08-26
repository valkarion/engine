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
#include "entityManager.hpp"
#include "sceneManager.hpp"
#include "camera.hpp"
#include "physicsSystem.hpp"

CVar window_width(	"window_width",		"1280" );
CVar window_height(	"window_height",	"800" );
CVar window_title(	"window_title",		"No Name Engine" );
CVar print_fps(		"print_fps",		"0" );

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

void Application::loadLuaData()
{
	// run user script files 
	std::vector<std::string> scripts = GetFilesInDirectory( "scripts", "lua" );
	for ( const auto& s : scripts )
	{
		LuaStateController::instance()->safeRunScriptFile( "scripts\\" + s + ".lua" );
	}

	// load scenes
	sol::table scenes = LuaStateController::instance()->getDataTable( "scenes" );
	if ( scenes )
	{
		for ( const auto& sc : scenes )
		{
			sol::table s = sc.second.as<sol::table>();
			const std::string name = s["name"].get_or<std::string>( UNSET_S );

			if ( name == UNSET_S )
			{
				WriteToErrorLog( "Scenes must have a unique name property; skipping." );
				continue;
			}						

			SC_ID id = SceneManager::instance()->addScene( name );		
			Scene* scene = SceneManager::instance()->getScene( id );
			scene->name = name;
			scene->worldObjName = s["world"].get_or<std::string>( UNSET_S );
		}
	}
}

bool Application::init()
{
	CVarSystem::instance()->registerStaticCVars();

	LuaStateController::instance()->state.open_libraries(
		sol::lib::base,	sol::lib::package, sol::lib::string,
		sol::lib::table, sol::lib::math );
	LuaStateController::instance()->registerFunctions();
	LuaStateController::instance()->registerClasses();

// Initialize lua Data table
	LuaStateController::instance()->safeRunScriptFile( "core\\data.lua" );
// Load user and core data 
	loadLuaData();

	initGLFW();

	EntityManager::instance()->initialize();

	Renderer::instance()->init();

// Load placeholder textures for renderer
	ResourceManager::instance()->loadImage( "core\\notexture.bmp", "notexture" );
	Renderer::instance()->loadTexture( "notexture" );

	InputSystem::instance()->init( Renderer::instance()->window );
	sol::table iTable = LuaStateController::instance()->getDataTable( "input" );
	InputSystem::instance()->setupInputFunctions( iTable );
	sol::table kTable = LuaStateController::instance()->getDataTable( "keymap" );
	InputSystem::instance()->setupKeyboardCommands( kTable );

	Camera::instance()->initCamera();

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
	frameCounter.update();

	while( !exitGame && !glfwWindowShouldClose( Renderer::instance()->window ) )
	{
	// Input 
		glfwPollEvents();
		InputSystem::instance()->update();

	// Systems
		PhysicsSystem::instance()->update( 
			frameCounter.lastFrameTimeInSeconds() );

		Camera::instance()->update();

	// Render
		Renderer::instance()->drawFrame();

		if ( print_fps.intValue == 1 )
		{
			SetWindowDebugTitle( Renderer::instance()->window, frameCounter.getFramerate() );
		}

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

	EntityManager::instance()->shutdown();

	glfwDestroyWindow( Renderer::instance()->window );
	glfwTerminate();
}