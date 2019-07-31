#include "tests.hpp"
#include "application.hpp"
#include "inputSystem.hpp"
#include "luaStateController.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "sceneManager.hpp"
#include "resourceManager.hpp"
#include "renderer.hpp"
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
	SceneManager*	sm	= SceneManager::instance();
	EntityManager*	em	= EntityManager::instance();
	ResourceManager* rm = ResourceManager::instance();
	Renderer* ren		= Renderer::instance();

// Load data to ResourceManager
	rm->loadImage( "D:\\engine\\textures\\dinosaur.bmp", "dinosaur" );
	rm->loadMesh( "D:\\engine\\models\\dinosaur.obj", "dinosaur" );

	rm->loadImage( "D:\\engine\\textures\\chalet.bmp", "chalet" );
	rm->loadMesh( "D:\\engine\\models\\chalet.obj", "chalet" );

// Convert that data to Renderable Stuff
	ren->loadTexture( "dinosaur" );
	ren->loadModel( "dinosaur" );

	ren->loadTexture( "chalet" );
	ren->loadModel( "chalet" );

// Set scene
	SC_ID sceneId = sm->addScene();
	Scene* currentScene = sm->setActiveScene( sceneId );
	
// Setup test Entities 
	{
		E_ID ent = em->addEntity();
		currentScene->entities.push_back( ent );

		TransformComponent* tc = em->add<TransformComponent>( ent );
		tc->position = glm::vec3( 0.f, 0.f, 0.f );
		tc->scale = glm::vec3( 0.1f );

		MeshComponent* mc = em->add<MeshComponent>( ent );
		mc->meshName = "dinosaur";
		mc->textureName = "dinosaur";
	}
	
	{
		E_ID ent = em->addEntity();
		currentScene->entities.push_back( ent );

		TransformComponent* tc = em->add<TransformComponent>( ent );
		tc->position = glm::vec3( 0.f, 0.f, 0.f );

		MeshComponent* mc = em->add<MeshComponent>( ent );
		mc->meshName = "chalet";
		mc->textureName = "chalet";

		tc->rotation.x = glm::radians( 270.f );
		tc->rotation.z = glm::radians( 90.f );

		tc->position.y = -1.f;
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