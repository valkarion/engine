#include "sceneManager.hpp"

std::unique_ptr<SceneManager> SceneManager::_instance = std::make_unique<SceneManager>();

SC_ID SceneManager::addScene( const std::string& name )
{
	SC_ID id = IDGET( SC_ID );
	scenes[id] = std::make_unique<Scene>();
	scenes[id]->id = id;

	namedScenes[name] = id;

	return id;
}

void SceneManager::destroyScene( SC_ID id )
{
	if ( scenes.count( id ) != 0 )
	{
		scenes.erase( id );
		IDFREE( id );
	}
}

Scene* SceneManager::getActiveScene()
{
	Scene* scene = nullptr;

	if ( activeScene != UNSET_ID )
	{
		return scenes.at( activeScene ).get();
	}

	return scene;
}

Scene* SceneManager::setActiveScene( SC_ID id )
{
	if ( scenes.count( id ) != 0 )
	{
		fireSceneEvents( enu_EVENT_TYPE::scene_leave );

		activeScene = id;
		
		fireSceneEvents( enu_EVENT_TYPE::scene_enter );
	}

	return getActiveScene();
}


Scene* SceneManager::getScene( SC_ID id )
{
	if ( scenes.count( id ) != 0 )
	{
		return scenes.at( id ).get();
	}

	return nullptr;
}

Scene* SceneManager::getScene( const std::string& name )
{
	auto iter = namedScenes.find( name );
	if ( iter != namedScenes.end() )
	{
		return scenes.at( iter->second ).get();
	}

	return nullptr;
}	

void SceneManager::fireSceneEvents( enu_EVENT_TYPE type )
{
	if ( activeScene != UNSET_ID )
	{
		Scene* sc = getActiveScene();

		if ( sc->sceneEvents.count( type ) != 0 )
		{
			for ( const EVENT_ID& id : sc->sceneEvents[type] )
			{
				EventManager::instance()->fire( id );
			}
		}
	}
}

void SceneManager::shutdown()
{
	activeScene = UNSET_ID;
	scenes.clear();
	IDRESET( SC_ID );
}

SceneManager* SceneManager::instance()
{
	return _instance.get();
}