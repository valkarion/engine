#pragma once

#include <vector>
#include "idManager.hpp"

/*
	Scenes are bascially maps, they contain entities, triggers 
	everything that can be displayed and interacted with. 
	The Scene can not only contain maps but also menus 
	eg.: the main menu of half life 2
*/
class Scene
{
public:
	std::vector<E_ID> entities;
};