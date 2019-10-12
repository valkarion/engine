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
#include "fileSystem.hpp"
#include <glm/glm.hpp>

void AddInputCommands()
{
	LuaStateController::instance()->safeRunScriptFile( "scripts\\data.lua" );
	LuaStateController::instance()->safeRunScriptFile( "scripts\\input.lua" );
	
	sol::table input = LuaStateController::instance()->getDataTable( "input" );
	sol::table keymap = LuaStateController::instance()->getDataTable( "keymap" );

	InputSystem::instance()->setupInputFunctions( input );
	InputSystem::instance()->setupInputCommands( keymap );
}

void SetupBasicScene()
{
	SceneManager*	sm = SceneManager::instance();
	EntityManager*	em = EntityManager::instance();
	ResourceManager* rm = ResourceManager::instance();
	Renderer* ren = Renderer::instance();

// Load data to ResourceManager
	rm->loadImage( "..\\textures\\chalet.jpg", "chalet" );
	rm->loadMesh( "..\\models\\chalet.obj", "chalet" );
	
// Convert that data to Renderable Stuff
	ren->loadTexture( "chalet" );
	ren->loadModel( "chalet" );

// Set scene
	SC_ID sceneId = sm->addScene( "scene" );
	Scene* currentScene = sm->setActiveScene( sceneId );

// Create Entity
	E_ID ent = em->addEntity();
	currentScene->entities.push_back( ent );

	TransformComponent* tc = em->add<TransformComponent>( ent );
	tc->position = glm::vec3( 0.f, 0.f, 0.f );
	tc->rotation = glm::vec3( 0.f, 0.f, 0.f );

	MeshComponent* mc = em->add<MeshComponent>( ent );
	mc->meshName = "chalet";
	mc->textureName = "chalet";
};

void SetupMapScene()
{
	SceneManager*	sm = SceneManager::instance();
	EntityManager*	em = EntityManager::instance();
	ResourceManager* rm = ResourceManager::instance();
	Renderer* ren = Renderer::instance();

	// Load data to ResourceManager
	rm->loadMesh( "..\\WS_WorkingDir\\models\\doom_E1M1.obj", "map", "..\\WS_WorkingDir\\models" );
	ren->loadModel( "map" );
	   
	std::vector<std::string> textures = FileSystem::GetFilesInDirectory( "..\\WS_WorkingDir\\textures", "png" );
	for ( const auto& t : textures )
	{
		char buffer[256];
		std::snprintf( buffer, 256, "..\\WS_WorkingDir\\textures\\%s.png", t.c_str() );
		rm->loadImage( buffer, t );

		ren->loadTexture( t );
	}
	
	// Set scene
	SC_ID sceneId = sm->addScene( "scene" );
	Scene* currentScene = sm->setActiveScene( sceneId );

	E_ID ent = em->addEntity();
	currentScene->world = ent;
	currentScene->entities.push_back( ent );

	TransformComponent* tc = em->add<TransformComponent>( ent );
	tc->position = glm::vec3( 0.f, 0.f, 0.f );

	MeshComponent* mc = em->add<MeshComponent>( ent );
	mc->meshName = "map";
}

void T_Renderer()
{
	if( !Application::instance()->init() )
	{		
		exit( -1 );
	}

	Camera::instance()->initCamera();
	AddInputCommands();
	
	
	//SetupBasicScene();
	SetupMapScene();

	E_ID world = SceneManager::instance()->getActiveScene()->world;

	MeshComponent* mesh = EntityManager::instance()->get<MeshComponent>( world );
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( world );

	glm::vec3 position = ResourceManager::instance()->getMesh( mesh->meshName )->vertecies[0].position;
	tc->scale = glm::vec3( 0.5f, 0.5f, 0.5f );

	Camera::instance()->setPosition( 0.5f * position );
	
	Application::instance()->run();
}