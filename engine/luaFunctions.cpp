#include "luaStateController.hpp"
#include "loggers.hpp"
#include "camera.hpp"
#include "cvar.hpp"
#include "cvarSystem.hpp"
#include "sceneManager.hpp"
#include "fileSystem.hpp"
#include "renderer.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "resourceManager.hpp"

extern CVar window_title;

// dev 
void DebugPrint( const std::string& message )
{
	PrintToOutputWindow( message );
}

void LoadAllTextures()
{
	std::vector<std::string> textures = GetFilesInDirectory( "textures", "png" );
	
	for ( const auto& t : textures )
	{
		char buffer[256];
		std::snprintf( buffer, 256, "textures\\%s.png", t.c_str() );
		
		if ( ResourceManager::instance()->loadImage( buffer, t ) )
		{
			Renderer::instance()->loadTexture( t );
		}
		else
		{
			WriteToErrorLog( "ResourceManager::loadImage failed to load image: %s", t.c_str() );
		}
	}
}

void LoadAllModels()
{
	std::vector<std::string> models = GetFilesInDirectory( "models", "obj" );
	
	for ( const auto& mdl : models )
	{
		char buffer[256];
		std::snprintf( buffer, 256, "models\\%s.obj", mdl.c_str() );

		if ( ResourceManager::instance()->loadMesh( buffer, mdl, "models" ) )
		{
			Renderer::instance()->loadModel( mdl );
		}
		else
		{
			WriteToErrorLog( "ResourceManager::loadMesh failed to load obj file: %s", mdl.c_str() );
		}
	}
}

// utils
void SetWindowTitle( const std::string& title )
{
	window_title.setValue( title );
}

void SetCVar( const std::string& cvar, sol::object value )
{
	CVar* cv = CVarSystem::instance()->find( cvar );
	if ( cv )
	{
		cv->setValue( value.as<std::string>() );
	}
}

// gameplay logic 
void SetActiveScene( const std::string& name )
{
	Scene* oldScene = SceneManager::instance()->getActiveScene();
	Scene* newScene = SceneManager::instance()->findSceneByName( name );

	if ( newScene )
	{
		EntityManager* em = EntityManager::instance();

		// destroy old world
		if ( oldScene != nullptr && oldScene->world != UNSET_ID )
		{
			em->removeEntity( oldScene->world );
		}

		// spawn new world
		if ( newScene->worldObjName != UNSET_S )
		{
			E_ID wId = EntityManager::instance()->addEntity();
			TransformComponent* tc = em->add<TransformComponent>( wId );
			MeshComponent* mc = em->add<MeshComponent>( wId );
			mc->meshName = newScene->worldObjName;
			newScene->world = wId;
			newScene->entities.push_back( wId );
		}
		
		SceneManager::instance()->setActiveScene( newScene->id );
	}
	else
	{
		WriteToErrorLog( "Failed to set active scene: %s", name.c_str() );
	}
}

// objects 
Camera* GetCamera()
{
	return Camera::instance();
}

void LuaStateController::registerFunctions()
{
	state["DebugPrint"] = DebugPrint;
	state["LoadAllModels"] = LoadAllModels;
	state["LoadAllTextures"] = LoadAllTextures;

	state["SetWindowName"] = SetWindowTitle;
	state["SetCVar"] = SetCVar;

	state["SetActiveScene"] = SetActiveScene;

	state["GetCamera"] = GetCamera;
}