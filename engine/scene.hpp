#pragma once

#include <vector>
#include "idManager.hpp"
#include "utils.hpp"
#include "eventManager.hpp"

/*
	Scenes are bascially maps, they contain entities, triggers 
	everything that can be displayed and interacted with. 
	The Scene can not only contain maps but also menus 
	eg.: the main menu of half life 2
*/
class Scene
{
public:
	SC_ID		id;
	std::string name;
	std::string	worldObjName;

	// world is a special multitexture entity
	E_ID world = UNSET_ID;

	std::vector<E_ID> entities;

	// scene specific events
	std::map<enu_EVENT_TYPE,
		std::vector<EVENT_ID>> sceneEvents;
};