#pragma once

#include "scene.hpp"
#include "utils.hpp"
#include <vector>
#include <map>
#include <memory>

class SceneManager
{
	static std::unique_ptr<SceneManager> _instance;

	SC_ID activeScene = UNSET_ID;

	std::map<SC_ID, std::unique_ptr<Scene>> scenes;
	std::map<std::string, SC_ID>			namedScenes;
public:
	SC_ID addScene( const std::string& name );
	void destroyScene( SC_ID id );
	Scene* getActiveScene();
	Scene* getSceneById( SC_ID id );
	Scene* setActiveScene( SC_ID id );
	Scene* findSceneByName( const std::string& name );

	void shutdown();

	static SceneManager* instance();
};