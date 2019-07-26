#include "tests.hpp"
#include "application.hpp"
#include "inputSystem.hpp"
#include "luaStateController.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "sceneManager.hpp"
#include "camera.hpp"
#include <glm/glm.hpp>

void AddInputCommands()
{
	LuaStateController::instance()->safeRunScriptFile( "scripts\\data.lua" );
	LuaStateController::instance()->safeRunScriptFile( "scripts\\input.lua" );
	
	sol::table input = LuaStateController::instance()->getDataTable( "input" );
	sol::table keymap = LuaStateController::instance()->getDataTable( "keymap" );

	InputSystem::instance()->setupInputFunctions( input );
	InputSystem::instance()->setupKeyboardCommands( keymap );
}

void SetupScene()
{
	SceneManager*	sm = SceneManager::instance();
	EntityManager*	em = EntityManager::instance();

// Set scene
	SC_ID sceneId = sm->addScene();
	Scene* currentScene = sm->setActiveScene( sceneId );
	
// Setup test Entities 
	//E_ID ent = em->addEntity();
	//TransformComponent* tc = em->add<TransformComponent>( ent );
	//tc->position = glm::vec3( -3.f, -3.f, 0.f );
	//currentScene->entities.push_back( ent );
	
	for ( size_t i = 0; i < 9; i++ )
	{
		E_ID ent = em->addEntity();
		currentScene->entities.push_back( ent );
		TransformComponent* tc = em->add<TransformComponent>( ent );
		
		int px = -3 + ( ( i % 3 ) * 3 );
		int py = -3 + ( ( i / 3 ) * 3 );
		tc->position = glm::vec3( float( px ), float( py ), 0.f );
	}
};

void T_Renderer()
{
	if( !Application::instance()->init() )
	{		
		exit( -1 );
	}

	Camera::instance()->initCamera();
	AddInputCommands();
	SetupScene();

	Application::instance()->run();
}